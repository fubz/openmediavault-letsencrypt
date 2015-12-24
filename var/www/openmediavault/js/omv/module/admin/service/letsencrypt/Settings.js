/**
 * Copyright (c) 2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.service.letsencrypt.Settings", {
    extend: "OMV.workspace.form.Panel",

    rpcService: "LetsEncrypt",
    rpcGetMethod: "getSettings",
    rpcSetMethod: "setSettings",

    getFormItems: function() {
        return [{
            xtype: "fieldset",
            title: _("LetsEncrypt settings"),
            defaults: {
                labelSeparator: ""
            },
            items: [{
                xtype: "checkbox",
                name: "enable",
                fieldLabel: _("Enable"),
                checked: false
            },{
				xtype: "textfield",
				name: "domain",
				fieldLabel: _("Domain"),
				allowBlank: false,
				value: location.hostname
				plugins: [{
					ptype: "fieldinfo",
					text: _("Domain that points to your server, e.g example.org, sub.afraid.org")
				}]
			}]
        },{
			xtype: "fieldset",
			title: _("Certificate Comment"),
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "cn",
				fieldLabel: _("Common Name"),
				allowBlank: false,
				value: location.hostname
			},{
				xtype: "textfield",
				name: "o",
				fieldLabel: _("Organization Name"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "ou",
				fieldLabel: _("Organizational Unit"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "l",
				fieldLabel: _("City"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "st",
				fieldLabel: _("State/Province"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "c",
				fieldLabel: _("Country"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "email",
				fieldLabel: _("Email"),
				allowBlank: true
				vtype: "email"
			}];
    }
});

OMV.WorkspaceManager.registerPanel({
    id: "settings",
    path: "/service/letsencrypt",
    text: _("Settings"),
    position: 10,
    className: "OMV.module.admin.service.letsencrypt.Settings"
});
