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

	requires: [
		"OMV.Rpc",
		"OMV.window.Execute"
	],
	
    rpcService: "LetsEncrypt",
    rpcGetMethod: "getSettings",
    rpcSetMethod: "setSettings",

    getButtonItems: function() {
        var items = this.callParent(arguments);

        items.push({
            id: this.getId() + "-generate",
            xtype: "button",
            text: _("Generate Certificate"),
            icon: "images/wrench.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            scope: this,
            handler: Ext.Function.bind(this.onGenerateButton, this)
        });

        return items;
    },

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
					plugins: [{
						ptype: "fieldinfo",
						text: _("Domain the certificate will be generated for and must point to this server, e.g example.org, sub.afraid.org")
					}]
				},{
					xtype: "textfield",
					name: "email",
					fieldLabel: _("Email"),
					allowBlank: false,
					vtype: "email",
					plugins: [{
						ptype: "fieldinfo",
						text: _("Required for registration with LetsEncrypt.org.  This email address can be used to recover lost certificates.")
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
				allowBlank: false
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
			}]
	    }
        //    ,{
        //    xtype: "fieldset",
        //    title: _("Certificate Generation"),
        //    defaults: {
        //        labelSeparator: ""
        //    },
        //    items: [{
        //        xtype   : "button",
        //        name    : "generate",
        //        text    : _("Generate Certificate"),
        //        scope   : this,
        //        handler : Ext.Function.bind(me.onGenerateButton, this),
        //        margin  : "5 0 0 0"
        //    }]
        //}
        ];
	},

    onGenerateButton: function() {
        var me = this;
        me.doSubmit();
        Ext.create("OMV.window.Execute", {
            title      : _("Generate"),
            rpcService : "LetsEncrypt",
            rpcMethod  : "generateCertificate",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    }

});

OMV.WorkspaceManager.registerPanel({
    id: "settings",
    path: "/service/letsencrypt",
    text: _("Settings"),
    position: 10,
    className: "OMV.module.admin.service.letsencrypt.Settings"
});
