/**
 * Copyright (c) 2015-2017 OpenMediaVault Plugin Developers
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

// require('js/omv/WorkspaceManager.js')
// require('js/omv/workspace/form/Panel.js')
// require('js/omv/Rpc.js')

Ext.define('OMV.module.admin.service.letsencrypt.Settings', {
    extend: 'OMV.workspace.form.Panel',

    requires: [
        'OMV.Rpc',
    ],

    rpcService: 'LetsEncrypt',
    rpcGetMethod: 'getSettings',
    rpcSetMethod: 'setSettings',

    getFormItems: function() {
        return [{
            xtype: 'fieldset',
            title: _('LetsEncrypt settings'),
            defaults: {
                labelSeparator: ''
            },
            items: [{
                xtype: 'checkbox',
                name: 'enable',
                fieldLabel: _('Schedule Refresh'),
                checked: false,
                boxLabel: _('Enable monthly update of certificate.  This will create a cron file in /etc/cron.d/.')
            },{
                xtype: 'checkbox',
                name: 'test_cert',
                fieldLabel: _('Test Certificate'),
                enable: false,
                checked: true,
                boxLabel: _('Do not enable until first certificate has been successfully generated. Once you have a certificate use this to avoid rate limit errors.')
            },{
                xtype: 'textfield',
                name: 'email',
                fieldLabel: _('Email'),
                allowBlank: false,
                vtype: 'email',
                plugins: [{
                    ptype: 'fieldinfo',
                    text: _('Required for registration with LetsEncrypt.org.  This email address can be used to recover lost certificates.')
                }]
            },{
                xtype: 'textfield',
                name: 'name',
                fieldLabel: _('Certificate Name'),
                allowBlank: false
            },{
                xtype: 'combo',
                name: 'keylength',
                fieldLabel: _('RSA Key Length'),
                mode: 'local',
                store: new Ext.data.SimpleStore({
                    fields: [ 'value', 'text' ],
                    data: [
                        [ 2048, _('2048') ],
                        [ 4096, _('4096') ]
                    ]
                }),
                displayField: 'text',
                valueField: 'value',
                allowBlank: false,
                editable: false,
                triggerAction: 'all',
                value: 2048,
                plugins: [{
                    ptype: 'fieldinfo',
                    text: _('Longer key lengths may cause initial ssl handshake to be significantly slower on low powered systems.')
                }]
            },{
                xtype: 'textfield',
                name: 'extraoptions',
                fieldLabel: _('Extra Options'),
                allowBlank: true
            },{
                xtype: 'hiddenfield',
                name: 'certuuid'
            }]
        },{
            xtype: 'fieldset',
            title: _('Tips'),
            defaults: {
                labelSeparator: ''
            },
            items: [{
                border: false,
                html: '<br><ul><li>Plugin uses <a href="https://letsencrypt.readthedocs.org/en/latest/using.html#webroot">the webroot installation</a> method provided by Let\'s Encrypt.</li>' +
                    '<li>OMV configuration needs to be applied after generating certificate due to the plugin adding entries to certificates and scheduled jobs.</li>' +
                    '<li>If you generate your first certificate with the test flag enabled your certificate will have an invalid root cert from Happy Hacker.  You will need to delete your /etc/letsencrypt folder and start over.</li>' +
                    '<h2>Server Setup</h2>' +
                    '<ul><li>Port <b>80</b> must be open for Let\'s Encrypt to verify your domain.</li>' +
                    '<li>Since the plugin only supports one webroot, use a reverse proxy to direct all of Let\'s Encrypt validation calls to yourdomain.tld/.well-known/acme-challenge/* to your configured webroot.</li>' +
                    '</ul>'
            }]
        }];
    }
});

OMV.WorkspaceManager.registerPanel({
    id: 'settings',
    path: '/service/letsencrypt',
    text: _('Settings'),
    position: 20,
    className: 'OMV.module.admin.service.letsencrypt.Settings'
});
