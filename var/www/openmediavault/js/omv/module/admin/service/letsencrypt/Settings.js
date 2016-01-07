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
					checked: false,
                    plugins: [{
                        ptype: "fieldinfo",
                        text: _("Enable monthly update of certificate.  This will create an entry in Scheduled Jobs.")
                    }]
				},{
					xtype: "textfield",
					name: "domain",
					fieldLabel: _("Domain"),
					allowBlank: false,
					plugins: [{
						ptype: "fieldinfo",
						text: _("Domain the certificate will be generated for and must point to this server, e.g example.org, sub.afraid.org.  Wildcard (*) domains are not supported.")
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
				},{
					xtype: "hiddenfield",
					name: "cron_uuid"
				},{
					xtype: "hiddenfield",
					name: "certuuid"
				}]
			},{
			xtype: "fieldset",
			title: _("Tips"),
			defaults: {
				labelSeparator: ""
			},
			items: [{
				border: false,
				html: "<h2>Server Setup</h2>" +
					"<ul><li>Port <b>80</b> must be open for Let's Encrypt to verify your domain.</li></ul>"
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
        var wnd = Ext.create("OMV.window.Execute", {
            title      			: _("Generate"),
            rpcService 			: "LetsEncrypt",
            rpcMethod  			: "generateCertificate",
			rpcIgnoreErrors 	: true,
			hideStartButton 	: true,
			hideStopButton  	: true,
			listeners  			: {
                scope     : me,
				finish    : function(wnd, response) {
					wnd.appendValue(_("Done..."));
					wnd.setButtonDisabled("close", false);
				},
				exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        });
		wnd.setButtonDisabled("close", true);
		wnd.show();
		wnd.start();
    }

});

OMV.WorkspaceManager.registerPanel({
    id: "settings",
    path: "/service/letsencrypt",
    text: _("Settings"),
    position: 10,
    className: "OMV.module.admin.service.letsencrypt.Settings"
});
