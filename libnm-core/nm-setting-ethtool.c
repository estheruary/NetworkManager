/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright 2018 Red Hat, Inc.
 */

#include "nm-default.h"

#include "nm-setting-ethtool.h"

#include "nm-setting-private.h"
#include "nm-libnm-core-intern/nm-ethtool-utils.h"

/*****************************************************************************/

/**
 * SECTION:nm-setting-ethtool
 * @short_description: Describes connection properties for ethtool related options
 *
 * The #NMSettingEthtool object is a #NMSetting subclass that describes properties
 * to control network driver and hardware settings.
 **/

/*****************************************************************************/

/**
 * nm_ethtool_optname_is_feature:
 * @optname: the option name to check
 *
 * Checks whether @optname is a valid option name for an offload feature.
 *
 * %Returns: %TRUE, if @optname is valid
 *
 * Since: 1.14
 */
gboolean
nm_ethtool_optname_is_feature (const char *optname)
{
	return optname && nm_ethtool_id_is_feature (nm_ethtool_id_get_by_name (optname));
}

/*****************************************************************************/

/**
 * NMSettingEthtool:
 *
 * Ethtool Ethernet Settings
 *
 * Since: 1.14
 */
struct _NMSettingEthtool {
	NMSetting parent;
};

struct _NMSettingEthtoolClass {
	NMSettingClass parent;
};

G_DEFINE_TYPE (NMSettingEthtool, nm_setting_ethtool, NM_TYPE_SETTING)

#define NM_SETTING_ETHTOOL_GET_PRIVATE(self) _NM_GET_PRIVATE (self, NMSettingEthtool, NM_IS_SETTING_ETHTOOL, NMSetting)

/*****************************************************************************/

static void
_notify_attributes (NMSettingEthtool *self)
{
	_nm_setting_gendata_notify (NM_SETTING (self), TRUE);
}

/*****************************************************************************/

/**
 * nm_setting_ethtool_get_feature:
 * @setting: the #NMSettingEthtool
 * @optname: option name of the offload feature to get
 *
 * Gets and offload feature setting. Returns %NM_TERNARY_DEFAULT if the
 * feature is not set.
 *
 * Returns: a #NMTernary value indicating whether the offload feature
 *   is enabled, disabled, or left untouched.
 *
 * Since: 1.14
 */
NMTernary
nm_setting_ethtool_get_feature (NMSettingEthtool *setting,
                                const char *optname)
{
	GVariant *v;

	g_return_val_if_fail (NM_IS_SETTING_ETHTOOL (setting), NM_TERNARY_DEFAULT);
	g_return_val_if_fail (optname && nm_ethtool_optname_is_feature (optname), NM_TERNARY_DEFAULT);

	v = nm_setting_gendata_get (NM_SETTING (setting), optname);
	if (   v
	    && g_variant_is_of_type (v, G_VARIANT_TYPE_BOOLEAN)) {
		return g_variant_get_boolean (v)
		       ? NM_TERNARY_TRUE
		       : NM_TERNARY_FALSE;
	}
	return NM_TERNARY_DEFAULT;
}

/**
 * nm_setting_ethtool_set_feature:
 * @setting: the #NMSettingEthtool
 * @optname: option name of the offload feature to get
 * @value: the new value to set. The special value %NM_TERNARY_DEFAULT
 *   means to clear the offload feature setting.
 *
 * Sets and offload feature setting.
 *
 * Since: 1.14
 */
void
nm_setting_ethtool_set_feature (NMSettingEthtool *setting,
                                const char *optname,
                                NMTernary value)
{
	GHashTable *hash;
	GVariant *v;

	g_return_if_fail (NM_IS_SETTING_ETHTOOL (setting));
	g_return_if_fail (optname && nm_ethtool_optname_is_feature (optname));
	g_return_if_fail (NM_IN_SET (value, NM_TERNARY_DEFAULT,
	                                    NM_TERNARY_FALSE,
	                                    NM_TERNARY_TRUE));

	hash = _nm_setting_gendata_hash (NM_SETTING (setting),
	                                 value != NM_TERNARY_DEFAULT);

	if (value == NM_TERNARY_DEFAULT) {
		if (hash) {
			if (g_hash_table_remove (hash, optname))
				_notify_attributes (setting);
		}
		return;
	}

	v = g_hash_table_lookup (hash, optname);
	if (   v
	    && g_variant_is_of_type (v, G_VARIANT_TYPE_BOOLEAN)) {
		if (g_variant_get_boolean (v)) {
			if (value == NM_TERNARY_TRUE)
				return;
		} else {
			if (value == NM_TERNARY_FALSE)
				return;
		}
	}

	v = g_variant_ref_sink (g_variant_new_boolean (value != NM_TERNARY_FALSE));
	g_hash_table_insert (hash,
	                     g_strdup (optname),
	                     v);
	_notify_attributes (setting);
}

/**
 * nm_setting_ethtool_clear_features:
 * @setting: the #NMSettingEthtool
 *
 * Clears all offload features settings
 *
 * Since: 1.14
 */
void
nm_setting_ethtool_clear_features (NMSettingEthtool *setting)
{
	GHashTable *hash;
	GHashTableIter iter;
	const char *name;
	gboolean changed = FALSE;

	g_return_if_fail (NM_IS_SETTING_ETHTOOL (setting));

	hash = _nm_setting_gendata_hash (NM_SETTING (setting), FALSE);
	if (!hash)
		return;

	g_hash_table_iter_init (&iter, hash);
	while (g_hash_table_iter_next (&iter, (gpointer *) &name, NULL)) {
		if (nm_ethtool_optname_is_feature (name)) {
			g_hash_table_iter_remove (&iter);
			changed = TRUE;
		}
	}

	if (changed)
		_notify_attributes (setting);
}

guint
nm_setting_ethtool_init_features (NMSettingEthtool *setting,
                                  NMTernary *requested /* indexed by NMEthtoolID - _NM_ETHTOOL_ID_FEATURE_FIRST */)
{
	GHashTable *hash;
	GHashTableIter iter;
	guint i;
	guint n_req = 0;
	const char *name;
	GVariant *variant;

	nm_assert (NM_IS_SETTING_ETHTOOL (setting));
	nm_assert (requested);

	for (i = 0; i < _NM_ETHTOOL_ID_FEATURE_NUM; i++)
		requested[i] = NM_TERNARY_DEFAULT;

	hash = _nm_setting_gendata_hash (NM_SETTING (setting), FALSE);
	if (!hash)
		return 0;

	g_hash_table_iter_init (&iter, hash);
	while (g_hash_table_iter_next (&iter, (gpointer *) &name, (gpointer *) &variant)) {
		NMEthtoolID ethtool_id = nm_ethtool_id_get_by_name (name);

		if (!nm_ethtool_id_is_feature (ethtool_id))
			continue;
		if (!g_variant_is_of_type (variant, G_VARIANT_TYPE_BOOLEAN))
			continue;

		requested[ethtool_id - _NM_ETHTOOL_ID_FEATURE_FIRST] = g_variant_get_boolean (variant)
		                                                       ? NM_TERNARY_TRUE
		                                                       : NM_TERNARY_FALSE;
		n_req++;
	}

	return n_req;
}

/*****************************************************************************/

static gboolean
verify (NMSetting *setting, NMConnection *connection, GError **error)
{
	GHashTable *hash;
	GHashTableIter iter;
	const char *optname;
	GVariant *variant;

	hash = _nm_setting_gendata_hash (setting, FALSE);

	if (!hash)
		goto out;

	g_hash_table_iter_init (&iter, hash);
	while (g_hash_table_iter_next (&iter, (gpointer *) &optname, (gpointer *) &variant)) {
		if (!nm_ethtool_optname_is_feature (optname)) {
			g_set_error_literal (error,
			                     NM_CONNECTION_ERROR,
			                     NM_CONNECTION_ERROR_INVALID_PROPERTY,
			                     _("unsupported offload feature"));
			g_prefix_error (error, "%s.%s: ", NM_SETTING_ETHTOOL_SETTING_NAME, optname);
			return FALSE;
		}
		if (!g_variant_is_of_type (variant, G_VARIANT_TYPE_BOOLEAN)) {
			g_set_error_literal (error,
			                     NM_CONNECTION_ERROR,
			                     NM_CONNECTION_ERROR_INVALID_PROPERTY,
			                     _("offload feature has invalid variant type"));
			g_prefix_error (error, "%s.%s: ", NM_SETTING_ETHTOOL_SETTING_NAME, optname);
			return FALSE;
		}
	}

out:
	return TRUE;
}

/*****************************************************************************/

static const GVariantType *
get_variant_type (const NMSettInfoSetting *sett_info,
                  const char *name,
                  GError **error)
{
	if (nm_ethtool_optname_is_feature (name))
		return G_VARIANT_TYPE_BOOLEAN;

	g_set_error (error,
	             NM_CONNECTION_ERROR,
	             NM_CONNECTION_ERROR_INVALID_PROPERTY,
	             _("unknown ethtool option '%s'"),
	             name);
	return NULL;
}

/*****************************************************************************/

static void
nm_setting_ethtool_init (NMSettingEthtool *setting)
{
}

/**
 * nm_setting_ethtool_new:
 *
 * Creates a new #NMSettingEthtool object with default values.
 *
 * Returns: (transfer full): the new empty #NMSettingEthtool object
 *
 * Since: 1.14
 **/
NMSetting *
nm_setting_ethtool_new (void)
{
	return g_object_new (NM_TYPE_SETTING_ETHTOOL, NULL);
}

static void
nm_setting_ethtool_class_init (NMSettingEthtoolClass *klass)
{
	NMSettingClass *setting_class = NM_SETTING_CLASS (klass);

	setting_class->verify = verify;

	_nm_setting_class_commit_full (setting_class,
	                               NM_META_SETTING_TYPE_ETHTOOL,
	                               NM_SETT_INFO_SETT_DETAIL (
	                                 .gendata_info = NM_SETT_INFO_SETT_GENDATA (
	                                     .get_variant_type = get_variant_type,
	                                 ),
	                               ),
	                               NULL);
}
