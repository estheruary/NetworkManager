/* nmcli - command-line tool to control NetworkManager
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright 2010 - 2017 Red Hat, Inc.
 */

#ifndef __NM_META_SETTING_DESC_H__
#define __NM_META_SETTING_DESC_H__

#include "nm-meta-setting.h"

#define NM_META_TEXT_HIDDEN "<hidden>"

typedef enum {
	NM_META_ACCESSOR_GET_TYPE_PRETTY,
	NM_META_ACCESSOR_GET_TYPE_PARSABLE,
} NMMetaAccessorGetType;

typedef enum {
	NM_META_ACCESSOR_GET_FLAGS_NONE                                         = 0,
	NM_META_ACCESSOR_GET_FLAGS_SHOW_SECRETS                                 = (1LL <<  0),
} NMMetaAccessorGetFlags;

typedef enum {
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PRETTY_NUMERIC                       = (1LL <<  0),
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PRETTY_NUMERIC_HEX                   = (1LL <<  1),
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PRETTY_TEXT                          = (1LL <<  2),
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PRETTY_TEXT_L10N                     = (1LL <<  3),
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PARSABLE_NUMERIC                     = (1LL <<  4),
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PARSABLE_NUMERIC_HEX                 = (1LL <<  5),
	NM_META_PROPERTY_TYP_FLAG_ENUM_GET_PARSABLE_TEXT                        = (1LL <<  6),
} NMMetaPropertyTypFlags;

typedef enum {
	NM_META_PROPERTY_TYPE_MAC_MODE_DEFAULT,
	NM_META_PROPERTY_TYPE_MAC_MODE_CLONED,
	NM_META_PROPERTY_TYPE_MAC_MODE_INFINIBAND,
} NMMetaPropertyTypeMacMode;

typedef struct _NMMetaEnvironment           NMMetaEnvironment;
typedef struct _NMMetaType                  NMMetaType;
typedef struct _NMMetaAbstractInfo          NMMetaAbstractInfo;
typedef struct _NMMetaSettingInfoEditor     NMMetaSettingInfoEditor;
typedef struct _NMMetaPropertyInfo          NMMetaPropertyInfo;
typedef struct _NMMetaPropertyType          NMMetaPropertyType;
typedef struct _NMMetaPropertyTypData       NMMetaPropertyTypData;

struct _NMMetaPropertyType {

	const char *(*describe_fcn) (const NMMetaPropertyInfo *property_info,
	                             char **out_to_free);

	char *(*get_fcn) (const NMMetaEnvironment *environment,
	                  gpointer environment_user_data,
	                  const NMMetaPropertyInfo *property_info,
	                  NMSetting *setting,
	                  NMMetaAccessorGetType get_type,
	                  NMMetaAccessorGetFlags get_flags);
	gboolean (*set_fcn) (const NMMetaEnvironment *environment,
	                     gpointer environment_user_data,
	                     const NMMetaPropertyInfo *property_info,
	                     NMSetting *setting,
	                     const char *value,
	                     GError **error);
	gboolean (*remove_fcn) (const NMMetaEnvironment *environment,
	                        gpointer environment_user_data,
	                        const NMMetaPropertyInfo *property_info,
	                        NMSetting *setting,
	                        const char *option,
	                        guint32 idx,
	                        GError **error);

	const char *const*(*values_fcn) (const NMMetaPropertyInfo *property_info,
	                                 char ***out_to_free);
};

struct _NMUtilsEnumValueInfo;

struct _NMMetaPropertyTypData {
	union {
		struct {
			gboolean (*fcn) (NMSetting *setting);
		} get_with_default;
		struct {
			GType (*get_gtype) (void);
			int min;
			int max;
			const struct _NMUtilsEnumValueInfo *value_infos;
		} gobject_enum;
		struct {
			guint32 (*get_fcn) (NMSetting *setting);
		} mtu;
		struct {
			NMMetaPropertyTypeMacMode mode;
		} mac;
	} subtype;
	const char *const*values_static;
	NMMetaPropertyTypFlags typ_flags;
};

struct _NMMetaPropertyInfo {
	const NMMetaType *meta_type;

	const NMMetaSettingInfoEditor *setting_info;

	const char *property_name;

	/* the property list for now must contain as first field the
	 * "name", which isn't a regular property. This is required by
	 * NmcOutputField and this first field is ignored for the
	 * group_list/setting_info. */
	bool is_name:1;

	bool is_secret:1;

	const char *describe_doc;

	const char *describe_message;

	const NMMetaPropertyType    *property_type;
	const NMMetaPropertyTypData *property_typ_data;
};

struct _NMMetaSettingInfoEditor {
	const NMMetaType *meta_type;
	const NMMetaSettingInfo *general;
	/* the order of the properties matter. The first *must* be the
	 * "name", and then the order is as they are listed by default. */
	const NMMetaPropertyInfo *properties;
	guint properties_num;
};

struct _NMMetaType {
	const char *type_name;
	const char *(*get_name) (const NMMetaAbstractInfo *abstract_info);
	const NMMetaAbstractInfo *const*(*get_nested) (const NMMetaAbstractInfo *abstract_info,
	                                               guint *out_len,
	                                               gpointer *out_to_free);
	gconstpointer (*get_fcn) (const NMMetaEnvironment *environment,
	                          gpointer environment_user_data,
	                          const NMMetaAbstractInfo *info,
	                          gpointer target,
	                          NMMetaAccessorGetType get_type,
	                          NMMetaAccessorGetFlags get_flags,
	                          gpointer *out_to_free);
};

struct _NMMetaAbstractInfo {
	union {
		const NMMetaType *meta_type;
		union {
			NMMetaSettingInfoEditor setting_info;
			NMMetaPropertyInfo property_info;
		} as;
	};
};

extern const NMMetaType nm_meta_type_setting_info_editor;
extern const NMMetaType nm_meta_type_property_info;

extern const NMMetaSettingInfoEditor nm_meta_setting_infos_editor[_NM_META_SETTING_TYPE_NUM];

/*****************************************************************************/

typedef enum {
	NM_META_ENV_WARN_LEVEL_INFO,
	NM_META_ENV_WARN_LEVEL_WARN,
} NMMetaEnvWarnLevel;

/* the settings-meta data is supposed to be independent of an actual client
 * implementation. Hence, there is a need for hooks to the meta-data.
 * The meta-data handlers may call back to the enviroment with certain
 * actions. */
struct _NMMetaEnvironment {

	void (*warn_fcn) (const NMMetaEnvironment *environment,
	                  gpointer environment_user_data,
	                  NMMetaEnvWarnLevel warn_level,
	                  const char *fmt_l10n, /* the untranslated format string, but it is marked for translation using N_(). */
	                  va_list ap);

};

/*****************************************************************************/

#endif /* __NM_META_SETTING_DESC_H__ */
