/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IGNITE_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW
#define _IGNITE_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW

#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/system/ui/custom_window.h"
// TODO: Removed these from configuration.h since no longer used. Moved here since they are still referenced. Remove when no longer needed.
#include "ignite/odbc/nested_tx_mode.h"
#include "ignite/odbc/protocol_version.h"
#include "ignite/odbc/ssl_mode.h"

namespace ignite
{
    namespace odbc
    {
        namespace system
        {
            namespace ui
            {
                /**
                 * DSN configuration window class.
                 */
                class DsnConfigurationWindow : public CustomWindow
                {
                    /**
                     * Children windows ids.
                     */
                    struct ChildId
                    {
                        enum Type
                        {
                            CONNECTION_SETTINGS_GROUP_BOX = 100,
                            SSH_SETTINGS_GROUP_BOX,
                            SSL_SETTINGS_GROUP_BOX,
                            ADDITIONAL_SETTINGS_GROUP_BOX,
                            AUTH_SETTINGS_GROUP_BOX,
                            NAME_EDIT,
                            NAME_LABEL,
                            ADDRESS_EDIT,
                            ADDRESS_LABEL,
                            SCHEMA_EDIT,
                            SCHEMA_LABEL,
                            SSH_USER_EDIT,
                            SSH_USER_LABEL,
                            SSH_HOST_EDIT,
                            SSH_HOST_LABEL,
                            SSH_PRIVATE_KEY_FILE_EDIT,
                            SSH_PRIVATE_KEY_FILE_LABEL,
                            SSH_PRIVATE_KEY_PASSPHRASE_EDIT,
                            SSH_PRIVATE_KEY_PASSPHRASE_LABEL,
                            SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX,
                            SSH_KNOWN_HOSTS_FILE_EDIT,
                            SSH_KNOWN_HOSTS_FILE_LABEL,
                            APP_NAME_EDIT,
                            APP_NAME_LABEL,
                            LOGIN_TIMEOUT_SEC_EDIT,
                            LOGIN_TIMEOUT_SEC_LABEL,
                            READ_PREFERENCE_LABEL,
                            READ_PREFERENCE_COMBO_BOX,
                            REPLICA_SET_EDIT,
                            REPLICA_SET_LABEL,
                            RETRY_READS_CHECK_BOX,
                            DEFAULT_FETCH_SIZE_EDIT,
                            DEFAULT_FETCH_SIZE_LABEL,
                            PROTOCOL_VERSION_LABEL,
                            PROTOCOL_VERSION_COMBO_BOX,
                            //NESTED_TX_MODE_LABEL,
                            //NESTED_TX_MODE_COMBO_BOX,
                            TLS_CHECK_BOX,
                            TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
                            TLS_CA_FILE_EDIT,
                            TLS_CA_FILE_LABEL,
                            USER_LABEL,
                            USER_EDIT,
                            PASSWORD_LABEL,
                            PASSWORD_EDIT,
                            OK_BUTTON,
                            CANCEL_BUTTON
                        };
                    };

                    // Window margin size.
                    enum { MARGIN = 10 };

                    // Standard interval between UI elements.
                    enum { INTERVAL = 10 };

                    // Standard row height.
                    enum { ROW_HEIGHT = 20 };

                    // Standard button width.
                    enum { BUTTON_WIDTH = 80 };

                    // Standard button height.
                    enum { BUTTON_HEIGHT = 25 };

                public:
                    /**
                     * Constructor.
                     *
                     * @param parent Parent window handle.
                     */
                    explicit DsnConfigurationWindow(Window* parent, config::Configuration& config);

                    /**
                     * Destructor.
                     */
                    virtual ~DsnConfigurationWindow();

                    /**
                     * Create window in the center of the parent window.
                     */
                    void Create();

                    /**
                    * @copedoc ignite::odbc::system::ui::CustomWindow::OnCreate
                    */
                    virtual void OnCreate();

                    /**
                     * @copedoc ignite::odbc::system::ui::CustomWindow::OnMessage
                     */
                    virtual bool OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

                private:
                    IGNITE_NO_COPY_ASSIGNMENT(DsnConfigurationWindow)

                    /**
                     * Retrieves current values from the children and stores
                     * them to the specified configuration.
                     *
                     * @param cfg Configuration.
                     */
                    void RetrieveParameters(config::Configuration& cfg) const;

                    /**
                     * Retrieves current values from the connection UI group and
                     * stores them to the specified configuration.
                     *
                     * @param cfg Configuration.
                     */
                    void RetrieveConnectionParameters(config::Configuration& cfg) const;

                    /**
                     * Retrieves current values from the Authentication UI group and
                     * stores them to the specified configuration.
                     *
                     * @param cfg Configuration.
                     */
                    void RetrieveAuthParameters(config::Configuration& cfg) const;

                    /**
                     * Retrieves current values from the SSH tunnel UI group and
                     * stores them to the specified configuration.
                     *
                     * @param cfg Configuration.
                     */
                    void RetrieveSshParameters(config::Configuration& cfg) const;

                    /**
                     * Retrieves current values from the SSL UI group and
                     * stores them to the specified configuration.
                     *
                     * @param cfg Configuration.
                     */
                    void RetrieveSslParameters(config::Configuration& cfg) const;

                    /**
                     * Retrieves current values from the additional UI group and
                     * stores them to the specified configuration.
                     *
                     * @param cfg Configuration.
                     */
                    void RetrieveAdditionalParameters(config::Configuration& cfg) const;

                    /**
                     * Create connection settings group box.
                     *
                     * @param posX X position.
                     * @param posY Y position.
                     * @param sizeX Width.
                     * @return Size by Y.
                     */
                    int CreateConnectionSettingsGroup(int posX, int posY, int sizeX);

                    /**
                     * Create authentication settings group box.
                     *
                     * @param posX X position.
                     * @param posY Y position.
                     * @param sizeX Width.
                     * @return Size by Y.
                     */
                    int CreateAuthSettingsGroup(int posX, int posY, int sizeX);

                     /**
                     * Create internal SSH tunnel settings group box.
                     *
                     * @param posX X position.
                     * @param posY Y position.
                     * @param sizeX Width.
                     * @return Size by Y.
                     */
                    int CreateSshSettingsGroup(int posX, int posY, int sizeX);

                    /**
                     * Create SSL settings group box.
                     *
                     * @param posX X position.
                     * @param posY Y position.
                     * @param sizeX Width.
                     * @return Size by Y.
                     */
                    int CreateSslSettingsGroup(int posX, int posY, int sizeX);

                    /**
                     * Create additional settings group box.
                     *
                     * @param posX X position.
                     * @param posY Y position.
                     * @param sizeX Width.
                     * @return Size by Y.
                     */
                    int CreateAdditionalSettingsGroup(int posX, int posY, int sizeX);

                    /** Window width. */
                    int width;

                    /** Window height. */
                    int height;

                    /** Connection settings group box. */
                    std::auto_ptr<Window> connectionSettingsGroupBox;

                    /** SSH settings group box. */
                    std::auto_ptr<Window> sshSettingsGroupBox;

                    /** SSL settings group box. */
                    std::auto_ptr<Window> sslSettingsGroupBox;

                    /** Authentication settings group box. */
                    std::auto_ptr<Window> authSettingsGroupBox;

                    /** Additional settings group box. */
                    std::auto_ptr<Window> additionalSettingsGroupBox;

                    /** DSN name edit field label. */
                    std::auto_ptr<Window> nameLabel;

                    /** DSN name edit field. */
                    std::auto_ptr<Window> nameEdit;

                    /** DSN address edit field label. */
                    std::auto_ptr<Window> addressLabel;

                    /** DSN address edit field. */
                    std::auto_ptr<Window> addressEdit;

                    /** DSN schema edit field label. */
                    std::auto_ptr<Window> schemaLabel;

                    /** DSN schema edit field. */
                    std::auto_ptr<Window> schemaEdit;

                    /** SSH user edit. */
                    std::auto_ptr<Window> sshUserEdit;

                    /** SSH user label. */
                    std::auto_ptr<Window> sshUserLabel;

                    /** SSH host edit. */
                    std::auto_ptr<Window> sshHostEdit;

                    /** SSH host label. */
                    std::auto_ptr<Window> sshHostLabel;

                    /** SSH private key file edit. */
                    std::auto_ptr<Window> sshPrivateKeyFileEdit;

                    /** SSH private key file label. */
                    std::auto_ptr<Window> sshPrivateKeyFileLabel;

                    /** SSH private key passphrase edit. */
                    std::auto_ptr<Window> sshPrivateKeyPassphraseEdit;

                    /** SSH private key passphrase label. */
                    std::auto_ptr<Window> sshPrivateKeyPassphraseLabel;

                    /** SSH strict host key checking checkBox. */
                    std::auto_ptr<Window> sshStrictHostKeyCheckingCheckBox;

                    /** SSH known host file edit. */
                    std::auto_ptr<Window> sshKnownHostsFileEdit;

                    /** SSH know host file label. */
                    std::auto_ptr<Window> sshKnownHostsFileLabel;

                    /** Application name edit. */
                    std::auto_ptr<Window> appNameEdit;

                    /** Application name label. */
                    std::auto_ptr<Window> appNameLabel;

                    /** Login Timeout (seconds) edit. */
                    std::auto_ptr<Window> loginTimeoutSecEdit;

                    /** Login Timeout (seconds) label. */
                    std::auto_ptr<Window> loginTimeoutSecLabel;

                    /** Nested Read Preference ComboBox **/
                    std::auto_ptr<Window> readPreferenceComboBox;

                    // -AL- remove later
                    ///** Read preference edit. */
                    //std::auto_ptr<Window> readPreferenceEdit;

                    /** Read preference label. */
                    std::auto_ptr<Window> readPreferenceLabel;

                    /** Replica set edit. */
                    std::auto_ptr<Window> replicaSetEdit;

                    /** Replica set label. */
                    std::auto_ptr<Window> replicaSetLabel;

                    /** Retry reads checkBox. */
                    std::auto_ptr<Window> retryReadsCheckBox;

                    /** Default fetch size edit. */
                    std::auto_ptr<Window> defaultFetchSizeEdit;

                    /** Default fetch size label. */
                    std::auto_ptr<Window> defaultFetchSizeLabel;

                    /** Protocol version edit field. */
                    std::auto_ptr<Window> protocolVersionLabel;

                    /** Protocol verion ComboBox. */
                    std::auto_ptr<Window> protocolVersionComboBox;

                    /** Ok button. */
                    std::auto_ptr<Window> okButton;

                    /** Cancel button. */
                    std::auto_ptr<Window> cancelButton;

                    /** TLS encryption checkBox. */
                    std::auto_ptr<Window> tlsCheckBox;

                    /** TLS allow invalid hostnames checkBox. */
                    std::auto_ptr<Window> tlsAllowInvalidHostnamesCheckBox;

                    /** TLS certificate authority file label. */
                    std::auto_ptr<Window> tlsCaFileLabel;

                    /** TLS certificate authority file edit. */
                    std::auto_ptr<Window> tlsCaFileEdit;

                    /** User label. */
                    std::auto_ptr<Window> userLabel;

                    /** User edit. */
                    std::auto_ptr<Window> userEdit;

                    /** Password label. */
                    std::auto_ptr<Window> passwordLabel;

                    /** Password edit. */
                    std::auto_ptr<Window> passwordEdit;

                    /** Configuration. */
                    config::Configuration& config;

                    /** Flag indicating whether OK option was selected. */
                    bool accepted;
                };
            }
        }
    }
}

#endif //_IGNITE_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW
