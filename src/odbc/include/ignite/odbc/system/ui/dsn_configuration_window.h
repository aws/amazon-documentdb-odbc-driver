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

namespace ignite {
namespace odbc {
namespace system {
namespace ui {
/**
 * DSN configuration window class.
 */
class DsnConfigurationWindow : public CustomWindow {
  /**
   * Children windows ids.
   */
  struct ChildId {
    enum Type {
      CONNECTION_SETTINGS_GROUP_BOX = 100,
      SSH_SETTINGS_GROUP_BOX,
      LOG_SETTINGS_GROUP_BOX,
      TLS_SETTINGS_GROUP_BOX,
      SCHEMA_SETTINGS_GROUP_BOX,
      ADDITIONAL_SETTINGS_GROUP_BOX,
      NAME_EDIT,
      NAME_LABEL,
      SSH_ENABLE_CHECK_BOX,
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
      LOG_LEVEL_LABEL,
      LOG_LEVEL_COMBO_BOX,
      LOG_PATH_LABEL,
      LOG_PATH_EDIT,
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
      SCAN_METHOD_COMBO_BOX,
      SCAN_METHOD_LABEL,
      SCAN_LIMIT_EDIT,
      SCAN_LIMIT_LABEL,
      SCHEMA_EDIT,
      SCHEMA_LABEL,
      REFRESH_SCHEMA_CHECK_BOX,
      TLS_CHECK_BOX,
      TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
      TLS_CA_FILE_EDIT,
      TLS_CA_FILE_LABEL,
      DRIVER_LABEL,
      DRIVER_EDIT,
      DATABASE_LABEL,
      DATABASE_EDIT,
      HOST_NAME_LABEL,
      HOST_NAME_EDIT,
      PORT_LABEL,
      PORT_EDIT,
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
  explicit DsnConfigurationWindow(Window* parent,
                                  config::Configuration& config);

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
   * Retrieves current values from the SSH tunnel UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveSshParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the log configuration UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveLogParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the TLS/SSL UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveTlsParameters(config::Configuration& cfg) const;

  /**
   * Retrieves current values from the schema generation UI group and
   * stores them to the specified configuration.
   *
   * @param cfg Configuration.
   */
  void RetrieveSchemaParameters(config::Configuration& cfg) const;

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
   * Create internal SSH tunnel settings group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateSshSettingsGroup(int posX, int posY, int sizeX);

  /**
   * Create logging configuration settings group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateLogSettingsGroup(int posX, int posY, int sizeX);

  /**
   * Create TLS/SSL settings group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateTlsSettingsGroup(int posX, int posY, int sizeX);

  /**
   * Create schema generation settings group box.
   *
   * @param posX X position.
   * @param posY Y position.
   * @param sizeX Width.
   * @return Size by Y.
   */
  int CreateSchemaSettingsGroup(int posX, int posY, int sizeX);

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
  std::unique_ptr< Window > connectionSettingsGroupBox;

  /** SSH settings group box. */
  std::unique_ptr< Window > sshSettingsGroupBox;

  /** Log settings group box. */
  std::unique_ptr< Window > logSettingsGroupBox;

  /** TLS settings group box. */
  std::unique_ptr< Window > tlsSettingsGroupBox;

  /** Schema generation and discovery settings group box. */
  std::unique_ptr< Window > schemaSettingsGroupBox;

  /** Additional settings group box. */
  std::unique_ptr< Window > additionalSettingsGroupBox;

  /** DSN name edit field label. */
  std::unique_ptr< Window > nameLabel;

  /** DSN name edit field. */
  std::unique_ptr< Window > nameEdit;

  /** Scan method ComboBox **/
  std::unique_ptr< Window > scanMethodComboBox;

  /** Scan method label. */
  std::unique_ptr< Window > scanMethodLabel;

  /** Scan limit field label. */
  std::unique_ptr< Window > scanLimitLabel;

  /** Scan limit field. */
  std::unique_ptr< Window > scanLimitEdit;

  /** DSN schema edit field label. */
  std::unique_ptr< Window > schemaLabel;

  /** DSN schema edit field. */
  std::unique_ptr< Window > schemaEdit;

  /** Refresh DSN schema checkBox. */
  std::unique_ptr< Window > refreshSchemaCheckBox;

  /** SSH enable checkBox. */
  std::unique_ptr< Window > sshEnableCheckBox;

  /** SSH user edit. */
  std::unique_ptr< Window > sshUserEdit;

  /** SSH user label. */
  std::unique_ptr< Window > sshUserLabel;

  /** SSH host edit. */
  std::unique_ptr< Window > sshHostEdit;

  /** SSH host label. */
  std::unique_ptr< Window > sshHostLabel;

  /** SSH private key file edit. */
  std::unique_ptr< Window > sshPrivateKeyFileEdit;

  /** SSH private key file label. */
  std::unique_ptr< Window > sshPrivateKeyFileLabel;

  /** SSH private key passphrase edit. */
  std::unique_ptr< Window > sshPrivateKeyPassphraseEdit;

  /** SSH private key passphrase label. */
  std::unique_ptr< Window > sshPrivateKeyPassphraseLabel;

  /** SSH strict host key checking checkBox. */
  std::unique_ptr< Window > sshStrictHostKeyCheckingCheckBox;

  /** SSH known host file edit. */
  std::unique_ptr< Window > sshKnownHostsFileEdit;

  /** SSH know host file label. */
  std::unique_ptr< Window > sshKnownHostsFileLabel;

  /** Log Level ComboBox **/
  std::unique_ptr< Window > logLevelComboBox;

  /** Log Level label. */
  std::unique_ptr< Window > logLevelLabel;

  /** Log Path edit. */
  std::unique_ptr< Window > logPathEdit;

  /** Log Path label. */
  std::unique_ptr< Window > logPathLabel;

  /** Application name edit. */
  std::unique_ptr< Window > appNameEdit;

  /** Application name label. */
  std::unique_ptr< Window > appNameLabel;

  /** Login Timeout (seconds) edit. */
  std::unique_ptr< Window > loginTimeoutSecEdit;

  /** Login Timeout (seconds) label. */
  std::unique_ptr< Window > loginTimeoutSecLabel;

  /** Read Preference ComboBox **/
  std::unique_ptr< Window > readPreferenceComboBox;

  /** Read preference label. */
  std::unique_ptr< Window > readPreferenceLabel;

  /** Replica set edit. */
  std::unique_ptr< Window > replicaSetEdit;

  /** Replica set label. */
  std::unique_ptr< Window > replicaSetLabel;

  /** Retry reads checkBox. */
  std::unique_ptr< Window > retryReadsCheckBox;

  /** Default fetch size edit. */
  std::unique_ptr< Window > defaultFetchSizeEdit;

  /** Default fetch size label. */
  std::unique_ptr< Window > defaultFetchSizeLabel;

  /** Ok button. */
  std::unique_ptr< Window > okButton;

  /** Cancel button. */
  std::unique_ptr< Window > cancelButton;

  /** TLS encryption checkBox. */
  std::unique_ptr< Window > tlsCheckBox;

  /** TLS allow invalid hostnames checkBox. */
  std::unique_ptr< Window > tlsAllowInvalidHostnamesCheckBox;

  /** TLS certificate authority file label. */
  std::unique_ptr< Window > tlsCaFileLabel;

  /** TLS certificate authority file edit. */
  std::unique_ptr< Window > tlsCaFileEdit;

  /** Database label. */
  std::unique_ptr< Window > databaseLabel;

  /** Database edit. */
  std::unique_ptr< Window > databaseEdit;

  /** Hostname label. */
  std::unique_ptr< Window > hostnameLabel;

  /** Hostname edit. */
  std::unique_ptr< Window > hostnameEdit;

  /** Port label. */
  std::unique_ptr< Window > portLabel;

  /** Port edit. */
  std::unique_ptr< Window > portEdit;

  /** User label. */
  std::unique_ptr< Window > userLabel;

  /** User edit. */
  std::unique_ptr< Window > userEdit;

  /** Password label. */
  std::unique_ptr< Window > passwordLabel;

  /** Password edit. */
  std::unique_ptr< Window > passwordEdit;

  /** Configuration. */
  config::Configuration& config;

  /** Flag indicating whether OK option was selected. */
  bool accepted;

  /** Flag indicating whether the configuration window has been created. */
  bool created;
};
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_SYSTEM_UI_DSN_CONFIGURATION_WINDOW
