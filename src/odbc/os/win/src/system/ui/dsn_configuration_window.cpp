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

#include "ignite/odbc/system/ui/dsn_configuration_window.h"

#include <Shlwapi.h>
#include <Windowsx.h>

#include "ignite/odbc/config/config_tools.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/log_level.h"
#include "ignite/odbc/read_preference.h"
#include "ignite/odbc/scan_method.h"

namespace ignite {
namespace odbc {
namespace system {
namespace ui {
DsnConfigurationWindow::DsnConfigurationWindow(Window* parent,
                                               config::Configuration& config)
    : CustomWindow(parent, L"IgniteConfigureDsn",
                   L"Configure Amazon DocumentDB DSN"),
      width(780),
      height(625),
      connectionSettingsGroupBox(),
      tlsSettingsGroupBox(),
      tlsCheckBox(),
      additionalSettingsGroupBox(),
      nameLabel(),
      nameEdit(),
      scanMethodLabel(),
      scanMethodComboBox(),
      scanLimitLabel(),
      scanLimitEdit(),
      schemaLabel(),
      schemaEdit(),
      refreshSchemaCheckBox(),
      sshEnableCheckBox(),
      sshUserLabel(),
      sshUserEdit(),
      sshHostLabel(),
      sshHostEdit(),
      sshPrivateKeyFileLabel(),
      sshPrivateKeyFileEdit(),
      sshPrivateKeyPassphraseLabel(),
      sshPrivateKeyPassphraseEdit(),
      sshStrictHostKeyCheckingCheckBox(),
      sshKnownHostsFileLabel(),
      sshKnownHostsFileEdit(),
      logLevelLabel(),
      logLevelComboBox(),
      logPathLabel(),
      logPathEdit(),
      appNameLabel(),
      appNameEdit(),
      readPreferenceLabel(),
      readPreferenceComboBox(),
      replicaSetLabel(),
      replicaSetEdit(),
      retryReadsCheckBox(),
      defaultFetchSizeLabel(),
      defaultFetchSizeEdit(),
      databaseLabel(),
      databaseEdit(),
      hostnameLabel(),
      hostnameEdit(),
      portLabel(),
      portEdit(),
      userLabel(),
      userEdit(),
      passwordLabel(),
      passwordEdit(),
      okButton(),
      cancelButton(),
      config(config),
      accepted(false),
      created(false) {
  // No-op.
}

DsnConfigurationWindow::~DsnConfigurationWindow() {
  // No-op.
}

void DsnConfigurationWindow::Create() {
  // Finding out parent position.
  RECT parentRect;
  GetWindowRect(parent->GetHandle(), &parentRect);

  // Positioning window to the center of parent window.
  const int posX =
      parentRect.left + (parentRect.right - parentRect.left - width) / 2;
  const int posY =
      parentRect.top + (parentRect.bottom - parentRect.top - height) / 2;

  RECT desiredRect = {posX, posY, posX + width, posY + height};
  AdjustWindowRect(&desiredRect,
                   WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, FALSE);

  Window::Create(WS_OVERLAPPED | WS_SYSMENU, desiredRect.left, desiredRect.top,
                 desiredRect.right - desiredRect.left,
                 desiredRect.bottom - desiredRect.top, 0);

  if (!handle) {
    std::stringstream buf;

    buf << "Can not create window, error code: " << GetLastError();

    throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
  }
}

void DsnConfigurationWindow::OnCreate() {
  int groupPosYLeft = MARGIN;
  int groupSizeY = width / 2 - 2 * MARGIN;
  int posXRight = width / 2 + MARGIN;
  int groupPosYRight = MARGIN;

  // create left column group settings
  groupPosYLeft +=
      INTERVAL
      + CreateConnectionSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
  groupPosYLeft +=
      INTERVAL + CreateTlsSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
  groupPosYLeft +=
      INTERVAL + CreateSchemaSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
  groupPosYLeft +=
      INTERVAL + CreateSshSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
  // create right column group settings
  groupPosYRight +=
      INTERVAL + CreateSshSettingsGroup(posXRight, groupPosYRight, groupSizeY);
  groupPosYRight +=
      INTERVAL + CreateLogSettingsGroup(posXRight, groupPosYRight, groupSizeY);
  groupPosYRight +=
      INTERVAL
      + CreateAdditionalSettingsGroup(posXRight, groupPosYRight, groupSizeY);

  int cancelPosX = width - MARGIN - BUTTON_WIDTH;
  int okPosX = cancelPosX - INTERVAL - BUTTON_WIDTH;

  okButton = CreateButton(okPosX, groupPosYRight, BUTTON_WIDTH, BUTTON_HEIGHT,
                          L"Ok", ChildId::OK_BUTTON);
  cancelButton = CreateButton(cancelPosX, groupPosYRight, BUTTON_WIDTH,
                              BUTTON_HEIGHT, L"Cancel", ChildId::CANCEL_BUTTON);

  // check whether the required fields are filled. If not, Ok button is
  // disabled.
  created = true;
  okButton->SetEnabled(nameEdit->HasText() && userEdit->HasText()
                       && passwordEdit->HasText() && databaseEdit->HasText()
                       && hostnameEdit->HasText() && portEdit->HasText());
}

int DsnConfigurationWindow::CreateConnectionSettingsGroup(int posX, int posY,
                                                          int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + 2 * INTERVAL;

  std::wstring val = utility::FromUtf8(config.GetDsn());
  nameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                          L"Data Source Name*:", ChildId::NAME_LABEL);
  nameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                        ChildId::NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetHostname());
  hostnameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Hostname*:", ChildId::HOST_NAME_LABEL);
  hostnameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                            val,
                            ChildId::HOST_NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring tmp = std::to_wstring(config.GetPort());
  val = tmp;
  portLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"Port*:",
                          ChildId::PORT_LABEL);
  portEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                        ChildId::PORT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetDatabase());
  databaseLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Database*:", ChildId::DATABASE_LABEL);
  databaseEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                            val,
                            ChildId::DATABASE_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetUser());
  userLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                          L"User* :", ChildId::USER_LABEL);
  userEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                        ChildId::USER_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetPassword());
  passwordLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Password*:", ChildId::PASSWORD_LABEL);
  passwordEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                            val,
                            ChildId::USER_EDIT, ES_PASSWORD);

  rowPos += INTERVAL + ROW_HEIGHT;

  connectionSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"Connection Settings",
                     ChildId::CONNECTION_SETTINGS_GROUP_BOX);

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateSshSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 120 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + 2 * INTERVAL;

  int checkBoxSize = sizeX - 2 * MARGIN;

  sshEnableCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, L"Enable SSH Tunnel",
      ChildId::SSH_ENABLE_CHECK_BOX, config.IsSshEnable());

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring val = utility::FromUtf8(config.GetSshUser());
  sshUserLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"SSH User:", ChildId::SSH_USER_LABEL);
  sshUserEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                           ChildId::SSH_USER_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetSshHost());
  sshHostLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"SSH Hostname:", ChildId::SSH_HOST_LABEL);
  sshHostEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                           ChildId::SSH_HOST_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetSshPrivateKeyFile());
  sshPrivateKeyFileLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"SSH Private Key File:",
                                       ChildId::SSH_PRIVATE_KEY_FILE_LABEL);
  sshPrivateKeyFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                     val, ChildId::SSH_PRIVATE_KEY_FILE_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetSshPrivateKeyPassphrase());
  // ssh private key passphrase label requires double the row height due to the
  // long label.
  sshPrivateKeyPassphraseLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT * 2,
                  L"SSH Private Key File Passphrase:",
                  ChildId::SSH_PRIVATE_KEY_PASSPHRASE_LABEL);
  sshPrivateKeyPassphraseEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                 ChildId::SSH_PRIVATE_KEY_PASSPHRASE_EDIT, ES_PASSWORD);

  rowPos += INTERVAL + ROW_HEIGHT;

  // SSH Strict Host Key Check check box needs to have editSizeX as size because
  // the string is long
  sshStrictHostKeyCheckingCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
      L"SSH Strict Host Key Check (disabling option is less secure)",
      ChildId::SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX,
      config.IsSshStrictHostKeyChecking());

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetSshKnownHostsFile());
  sshKnownHostsFileLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"SSH Known Hosts File:",
                                       ChildId::SSH_KNOWN_HOSTS_FILE_LABEL);
  sshKnownHostsFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                     val, ChildId::SSH_KNOWN_HOSTS_FILE_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  sshSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                                       L"Internal SSH Tunnel Settings",
                                       ChildId::SSH_SETTINGS_GROUP_BOX);

  sshUserEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshHostEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshPrivateKeyFileEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshPrivateKeyPassphraseEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshStrictHostKeyCheckingCheckBox->SetEnabled(sshEnableCheckBox->IsChecked());
  sshKnownHostsFileEdit->SetEnabled(
      sshEnableCheckBox->IsChecked()
      && sshStrictHostKeyCheckingCheckBox->IsChecked());

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateLogSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 120 };

  int labelPosX = posX + INTERVAL;
  int pathSizeX = sizeX - 2 * INTERVAL;
  int comboSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX;

  int rowPos = posY + 2 * INTERVAL;

  LogLevel::Type logLevel = config.GetLogLevel();

  logLevelLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Log Level:", ChildId::LOG_LEVEL_LABEL);
  logLevelComboBox = CreateComboBox(editPosX, rowPos, comboSizeX, ROW_HEIGHT,
                                    L"",
                                    ChildId::LOG_LEVEL_COMBO_BOX);

  logLevelComboBox->AddString(L"Debug");
  logLevelComboBox->AddString(L"Info");
  logLevelComboBox->AddString(L"Error");
  logLevelComboBox->AddString(L"Off");

  logLevelComboBox->SetSelection(static_cast< int >(logLevel));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring val = utility::FromUtf8(config.GetLogPath());
  logPathLabel = CreateLabel(
      labelPosX, rowPos, pathSizeX, ROW_HEIGHT * 2,
      L"Log Path:\n(the log file name format is docdb_odbc_YYYYMMDD.log)",
      ChildId::LOG_PATH_LABEL);

  rowPos += INTERVAL * 2 + ROW_HEIGHT;

  logPathEdit = CreateEdit(editPosX, rowPos, pathSizeX, ROW_HEIGHT, val,
                           ChildId::LOG_PATH_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  logSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"Log Settings",
                     ChildId::LOG_SETTINGS_GROUP_BOX);

  std::wstring logLevelStr;
  logLevelComboBox->GetText(logLevelStr);
  if (LogLevel::FromString(utility::ToUtf8(logLevelStr), LogLevel::Type::UNKNOWN)
      == LogLevel::Type::OFF) {
    logPathEdit->SetEnabled(false);
  } else {
    logPathEdit->SetEnabled(true);
  }

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateTlsSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + 2 * INTERVAL;

  int checkBoxSize = sizeX - 2 * MARGIN;

  tlsCheckBox =
      CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, L"Enable TLS",
                     ChildId::TLS_CHECK_BOX, config.IsTls());

  rowPos += INTERVAL + ROW_HEIGHT;

  tlsAllowInvalidHostnamesCheckBox =
      CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                     L"Allow Invalid Hostnames (enabling option is less secure)",
                     ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
                     config.IsTlsAllowInvalidHostnames());

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring val = utility::FromUtf8(config.GetTlsCaFile());
  tlsCaFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                               L"TLS CA File:", ChildId::TLS_CA_FILE_LABEL);
  tlsCaFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                             ChildId::TLS_CA_FILE_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  tlsSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"TLS/SSL Settings",
                     ChildId::TLS_SETTINGS_GROUP_BOX);

  tlsAllowInvalidHostnamesCheckBox->SetEnabled(tlsCheckBox->IsChecked());
  tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateSchemaSettingsGroup(int posX, int posY,
                                                      int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + 2 * INTERVAL;

  int checkBoxSize = sizeX - 2 * MARGIN;

  ScanMethod::Type scanMethod = config.GetScanMethod();

  scanMethodLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                L"Scan Method:", ChildId::SCAN_METHOD_LABEL);
  scanMethodComboBox = CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                      L"", ChildId::SCAN_METHOD_COMBO_BOX);

  scanMethodComboBox->AddString(L"Random");
  scanMethodComboBox->AddString(L"ID Forward");
  scanMethodComboBox->AddString(L"ID Reverse");
  scanMethodComboBox->AddString(L"All");

  scanMethodComboBox->SetSelection(
      static_cast< int >(scanMethod));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring tmp = std::to_wstring(config.GetScanLimit());
  std::wstring val = tmp.c_str();
  scanLimitLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                               L"Scan Limit:", ChildId::SCAN_LIMIT_LABEL);
  scanLimitEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                             ChildId::SCAN_LIMIT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetSchemaName());
  schemaLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                            L"Schema Name:", ChildId::SCHEMA_LABEL);
  schemaEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                          ChildId::SCHEMA_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  refreshSchemaCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
      L"Refresh Schema (Caution: use temporarily to update schema)",
      ChildId::REFRESH_SCHEMA_CHECK_BOX, config.IsRefreshSchema());

  rowPos += INTERVAL + ROW_HEIGHT;

  schemaSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                                          L"Schema Generation Settings",
                                          ChildId::SCHEMA_SETTINGS_GROUP_BOX);

  std::wstring scanMethodStr;
  scanMethodComboBox->GetText(scanMethodStr);
  if (ScanMethod::FromString(utility::ToUtf8(scanMethodStr), ScanMethod::Type::UNKNOWN)
      == ScanMethod::Type::ALL) {
    scanLimitEdit->SetEnabled(false);
  } else {
    scanLimitEdit->SetEnabled(true);
  }

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateAdditionalSettingsGroup(int posX, int posY,
                                                          int sizeX) {
  enum { LABEL_WIDTH = 120 };  // same as SSH settings

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int checkBoxSize = (sizeX - 3 * INTERVAL) / 2;

  int rowPos = posY + 2 * INTERVAL;

  retryReadsCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, L"Retry Reads",
                     ChildId::RETRY_READS_CHECK_BOX, config.IsRetryReads());

  rowPos += INTERVAL + ROW_HEIGHT;

  ReadPreference::Type readPreference = config.GetReadPreference();

  readPreferenceLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Read preference:", ChildId::READ_PREFERENCE_LABEL);
  readPreferenceComboBox =
      CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT, L"",
                     ChildId::READ_PREFERENCE_COMBO_BOX);

  readPreferenceComboBox->AddString(L"Primary");
  readPreferenceComboBox->AddString(L"Primary Preferred");
  readPreferenceComboBox->AddString(L"Secondary");
  readPreferenceComboBox->AddString(L"Secondary Preferred");
  readPreferenceComboBox->AddString(L"Nearest");

  readPreferenceComboBox->SetSelection(
      static_cast< int >(readPreference));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring val = utility::FromUtf8(config.GetApplicationName());
  appNameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"Application Name:", ChildId::APP_NAME_LABEL);
  appNameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                           ChildId::APP_NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring tmp =
      std::to_wstring(config.GetLoginTimeoutSeconds());
  val = tmp;
  loginTimeoutSecLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Login Timeout (s):", ChildId::LOGIN_TIMEOUT_SEC_LABEL);

  loginTimeoutSecEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::LOGIN_TIMEOUT_SEC_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  val = utility::FromUtf8(config.GetReplicaSet());
  replicaSetLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                L"Replica Set:", ChildId::REPLICA_SET_LABEL);
  replicaSetEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                              ChildId::REPLICA_SET_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  tmp = std::to_wstring(config.GetDefaultFetchSize());
  val = tmp;
  defaultFetchSizeLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Fetch Size:", ChildId::DEFAULT_FETCH_SIZE_LABEL);

  defaultFetchSizeEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                 ChildId::DEFAULT_FETCH_SIZE_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  additionalSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"Additional Settings",
                     ChildId::ADDITIONAL_SETTINGS_GROUP_BOX);

  return rowPos - posY;
}

bool DsnConfigurationWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case ChildId::OK_BUTTON: {
          try {
            RetrieveParameters(config);

            accepted = true;

            PostMessage(GetHandle(), WM_CLOSE, 0, 0);
          } catch (IgniteError& err) {
            std::wstring errText = utility::FromUtf8(err.GetText());
            MessageBox(NULL, errText.c_str(), L"Error!",
                       MB_ICONEXCLAMATION | MB_OK);
          }

          break;
        }

        case IDCANCEL:
        case ChildId::CANCEL_BUTTON: {
          PostMessage(GetHandle(), WM_CLOSE, 0, 0);

          break;
        }

        case ChildId::NAME_EDIT:
        case ChildId::HOST_NAME_EDIT:
        case ChildId::PORT_EDIT:
        case ChildId::DATABASE_EDIT:
        case ChildId::USER_EDIT:
        case ChildId::PASSWORD_EDIT: {
          // Check if window has been created.
          if (created) {
            okButton->SetEnabled(
                nameEdit->HasText() && userEdit->HasText()
                && passwordEdit->HasText() && databaseEdit->HasText()
                && hostnameEdit->HasText() && portEdit->HasText());
          }
          break;
        }

        case ChildId::SSH_ENABLE_CHECK_BOX: {
          sshEnableCheckBox->SetChecked(!sshEnableCheckBox->IsChecked());
          sshUserEdit->SetEnabled(sshEnableCheckBox->IsChecked());
          sshHostEdit->SetEnabled(sshEnableCheckBox->IsChecked());
          sshPrivateKeyFileEdit->SetEnabled(sshEnableCheckBox->IsChecked());
          sshPrivateKeyPassphraseEdit->SetEnabled(
              sshEnableCheckBox->IsChecked());
          sshStrictHostKeyCheckingCheckBox->SetEnabled(
              sshEnableCheckBox->IsChecked());
          sshKnownHostsFileEdit->SetEnabled(
              sshEnableCheckBox->IsChecked()
              && sshStrictHostKeyCheckingCheckBox->IsChecked());

          break;
        }

        case ChildId::SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX: {
          sshStrictHostKeyCheckingCheckBox->SetChecked(
              !sshStrictHostKeyCheckingCheckBox->IsChecked());
          sshKnownHostsFileEdit->SetEnabled(
              sshEnableCheckBox->IsChecked()
              && sshStrictHostKeyCheckingCheckBox->IsChecked());
          break;
        }

        case ChildId::TLS_CHECK_BOX: {
          tlsCheckBox->SetChecked(!tlsCheckBox->IsChecked());
          tlsAllowInvalidHostnamesCheckBox->SetEnabled(
              tlsCheckBox->IsChecked());
          tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());

          break;
        }

        case ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX: {
          tlsAllowInvalidHostnamesCheckBox->SetChecked(
              !tlsAllowInvalidHostnamesCheckBox->IsChecked());

          break;
        }

        case ChildId::LOG_LEVEL_COMBO_BOX: {
          std::wstring logLevelStr;
          logLevelComboBox->GetText(logLevelStr);
          if (LogLevel::FromString(utility::ToUtf8(logLevelStr), LogLevel::Type::UNKNOWN)
              == LogLevel::Type::OFF) {
            logPathEdit->SetEnabled(false);
          } else {
            logPathEdit->SetEnabled(true);
          }
          break;
        }

        case ChildId::SCAN_METHOD_COMBO_BOX: {
          std::wstring scanMethodStr;
          scanMethodComboBox->GetText(scanMethodStr);
          if (ScanMethod::FromString(utility::ToUtf8(scanMethodStr), ScanMethod::Type::UNKNOWN)
              == ScanMethod::Type::ALL) {
            scanLimitEdit->SetEnabled(false);
          } else {
            scanLimitEdit->SetEnabled(true);
          }
          break;
        }

        case ChildId::REFRESH_SCHEMA_CHECK_BOX: {
          refreshSchemaCheckBox->SetChecked(
              !refreshSchemaCheckBox->IsChecked());
          break;
        }

        case ChildId::RETRY_READS_CHECK_BOX: {
          retryReadsCheckBox->SetChecked(!retryReadsCheckBox->IsChecked());

          break;
        }

        default:
          return false;
      }

      break;
    }

    case WM_DESTROY: {
      PostQuitMessage(accepted ? Result::OK : Result::CANCEL);

      break;
    }

    default:
      return false;
  }

  return true;
}

void DsnConfigurationWindow::RetrieveParameters(
    config::Configuration& cfg) const {
  RetrieveLogParameters(cfg);
  RetrieveConnectionParameters(cfg);
  RetrieveSshParameters(cfg);
  RetrieveTlsParameters(cfg);
  RetrieveSchemaParameters(cfg);
  RetrieveAdditionalParameters(cfg);
}

void DsnConfigurationWindow::RetrieveConnectionParameters(
    config::Configuration& cfg) const {
  std::wstring dsnStr;
  std::wstring hostnameStr;
  std::wstring portStr;
  std::wstring databaseStr;
  std::wstring userStr;
  std::wstring passwordStr;

  nameEdit->GetText(dsnStr);
  std::string dsnStr0 = utility::ToUtf8(dsnStr);
  common::StripSurroundingWhitespaces(dsnStr0);
  // Stripping of whitespaces off the schema skipped intentionally

  hostnameEdit->GetText(hostnameStr);
  portEdit->GetText(portStr);
  databaseEdit->GetText(databaseStr);
  userEdit->GetText(userStr);
  passwordEdit->GetText(passwordStr);

  std::string hostnameStr0 = utility::ToUtf8(hostnameStr);
  std::string portStr0 = utility::ToUtf8(portStr);
  std::string databaseStr0 = utility::ToUtf8(databaseStr);
  std::string userStr0 = utility::ToUtf8(userStr);
  std::string passwordStr0 = utility::ToUtf8(passwordStr);

  int16_t port = common::LexicalCast< int16_t >(portStr0);

  if (port <= 0)
    port = config.GetPort();

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("DSN:      " << dsnStr0);
  LOG_MSG("Hostname: " << hostnameStr0);
  LOG_MSG("Port:     " << portStr0);
  LOG_MSG("Database: " << databaseStr0);

  // username and password intentionally not logged for security reasons

  cfg.SetDsn(dsnStr0);
  cfg.SetPort(port);
  cfg.SetHostname(hostnameStr0);
  cfg.SetDatabase(databaseStr0);
  cfg.SetUser(userStr0);
  cfg.SetPassword(passwordStr0);
}

void DsnConfigurationWindow::RetrieveSshParameters(
    config::Configuration& cfg) const {
  bool sshEnable = sshEnableCheckBox->IsChecked();
  bool sshStrictHostKeyChecking = sshStrictHostKeyCheckingCheckBox->IsChecked();

  std::wstring sshUserStr;
  std::wstring sshHostStr;
  std::wstring sshPrivateKeyFileStr;
  std::wstring sshPrivateKeyPassphraseStr;
  std::wstring sshKnownHostsFileStr;

  sshUserEdit->GetText(sshUserStr);
  sshHostEdit->GetText(sshHostStr);
  sshPrivateKeyFileEdit->GetText(sshPrivateKeyFileStr);
  sshPrivateKeyPassphraseEdit->GetText(sshPrivateKeyPassphraseStr);
  sshKnownHostsFileEdit->GetText(sshKnownHostsFileStr);

  std::string sshUserStr0 = utility::ToUtf8(sshUserStr);
  std::string sshHostStr0 = utility::ToUtf8(sshHostStr);
  std::string sshPrivateKeyFileStr0 = utility::ToUtf8(sshPrivateKeyFileStr);
  std::string sshPrivateKeyPassphraseStr0 = utility::ToUtf8(sshPrivateKeyPassphraseStr);
  std::string sshKnownHostsFileStr0 = utility::ToUtf8(sshKnownHostsFileStr);

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("SSH enable:                    " << (sshEnable ? "true" : "false"));
  LOG_MSG("SSH user:                      " << sshUserStr0);
  LOG_MSG("SSH host:                      " << sshHostStr0);
  LOG_MSG("SSH private key file:          " << sshPrivateKeyFileStr0);
  LOG_MSG("SSH strict host key checking:  "
          << (sshStrictHostKeyChecking ? "true" : "false"));
  LOG_MSG("SSH known hosts file:          " << sshKnownHostsFileStr0);

  cfg.SetSshEnable(sshEnable);
  cfg.SetSshUser(sshUserStr0);
  cfg.SetSshHost(sshHostStr0);
  cfg.SetSshPrivateKeyFile(sshPrivateKeyFileStr0);
  cfg.SetSshPrivateKeyPassphrase(sshPrivateKeyPassphraseStr0);
  cfg.SetSshStrictHostKeyChecking(sshStrictHostKeyChecking);
  cfg.SetSshKnownHostsFile(sshKnownHostsFileStr0);
}

// RetrieveLogParameters is a special case. We want to get the log level and
// path as soon as possible. If user set log level to OFF, then nothing should
// be logged. Therefore, the LOG_MSG calls are after log level and log path are
// set.
void DsnConfigurationWindow::RetrieveLogParameters(
    config::Configuration& cfg) const {
  std::wstring logLevelStr;
  std::wstring logPathStr;

  logLevelComboBox->GetText(logLevelStr);
  logPathEdit->GetText(logPathStr);

  std::string logLevelStr0 = utility::ToUtf8(logLevelStr);
  std::string logPathStr0 = utility::ToUtf8(logPathStr);

  LogLevel::Type logLevel =
      LogLevel::FromString(logLevelStr0, LogLevel::Type::UNKNOWN);

  cfg.SetLogLevel(logLevel);
  cfg.SetLogPath(logPathStr0);

  LOG_MSG("Log level:    " << logLevelStr0);
  LOG_MSG("Log path:     " << logPathStr0);
}

void DsnConfigurationWindow::RetrieveTlsParameters(
    config::Configuration& cfg) const {
  bool tls = tlsCheckBox->IsChecked();
  bool tlsAllowInvalidHostnames = tlsAllowInvalidHostnamesCheckBox->IsChecked();
  std::wstring tlsCaStr;

  tlsCaFileEdit->GetText(tlsCaStr);

  std::string tlsCaStr0 = utility::ToUtf8(tlsCaStr);

  LOG_MSG(
      "TLS/SSL Encryption:                       " << (tls ? "true" : "false"));
  LOG_MSG("TLS Allow Invalid Hostnames:              "
          << (tlsAllowInvalidHostnames ? "true" : "false"));
  LOG_MSG("TLS CA (Certificate Authority) File: " << tlsCaStr0);

  cfg.SetTls(tls);
  cfg.SetTlsAllowInvalidHostnames(tlsAllowInvalidHostnames);
  cfg.SetTlsCaFile(tlsCaStr0);
}

void DsnConfigurationWindow::RetrieveSchemaParameters(
    config::Configuration& cfg) const {
  std::wstring scanMethodStr;
  std::wstring scanLimitStr;
  std::wstring schemaStr;
  bool refreshSchema = refreshSchemaCheckBox->IsChecked();

  scanMethodComboBox->GetText(scanMethodStr);
  scanLimitEdit->GetText(scanLimitStr);
  schemaEdit->GetText(schemaStr);

  std::string scanMethodStr0 = utility::ToUtf8(scanMethodStr);
  std::string scanLimitStr0 = utility::ToUtf8(scanLimitStr);
  std::string schemaStr0 = utility::ToUtf8(schemaStr);

  int32_t scanLimit = common::LexicalCast< int32_t >(scanLimitStr0);

  if (scanLimit <= 0)
    scanLimit = config.GetScanLimit();

  LOG_MSG("Scan method:    " << scanMethodStr0);
  LOG_MSG("Scan limit      " << scanLimit);
  LOG_MSG("Schema:         " << schemaStr0);
  LOG_MSG("Refresh schema: " << (refreshSchema ? "true" : "false"));

  ScanMethod::Type scanMethod =
      ScanMethod::FromString(scanMethodStr0, ScanMethod::Type::UNKNOWN);

  cfg.SetScanMethod(scanMethod);
  cfg.SetSchemaName(schemaStr0);
  cfg.SetScanLimit(scanLimit);
  cfg.SetRefreshSchema(refreshSchema);
}

void DsnConfigurationWindow::RetrieveAdditionalParameters(
    config::Configuration& cfg) const {
  std::wstring readPreferenceStr;
  std::wstring appNameStr;
  std::wstring replicaSetStr;
  std::wstring loginTimeoutSecStr;
  std::wstring fetchSizeStr;

  readPreferenceComboBox->GetText(readPreferenceStr);
  appNameEdit->GetText(appNameStr);
  replicaSetEdit->GetText(replicaSetStr);
  bool retryReads = retryReadsCheckBox->IsChecked();
  loginTimeoutSecEdit->GetText(loginTimeoutSecStr);
  defaultFetchSizeEdit->GetText(fetchSizeStr);

  std::string readPreferenceStr0 = utility::ToUtf8(readPreferenceStr);
  std::string appNameStr0 = utility::ToUtf8(appNameStr);
  std::string replicaSetStr0 = utility::ToUtf8(replicaSetStr);
  std::string loginTimeoutSecStr0 = utility::ToUtf8(loginTimeoutSecStr);
  std::string fetchSizeStr0 = utility::ToUtf8(fetchSizeStr);

  int32_t loginTimeoutSec = common::LexicalCast< int32_t >(loginTimeoutSecStr0);
  if (loginTimeoutSec <= 0)
    loginTimeoutSec = config.GetLoginTimeoutSeconds();

  int32_t fetchSize = common::LexicalCast< int32_t >(fetchSizeStr0);
  if (fetchSize <= 0)
    fetchSize = config.GetDefaultFetchSize();

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("Retry reads:              " << (retryReads ? "true" : "false"));
  LOG_MSG("Read preference:          " << readPreferenceStr0);
  LOG_MSG("App name:                 " << appNameStr0);
  LOG_MSG("Login timeout (seconds):  " << loginTimeoutSecStr0);
  LOG_MSG("Replica Set:              " << replicaSetStr0);
  LOG_MSG("Fetch size:               " << fetchSize);

  ReadPreference::Type readPreference = ReadPreference::FromString(
      readPreferenceStr0, ReadPreference::Type::UNKNOWN);

  cfg.SetReadPreference(readPreference);
  cfg.SetRetryReads(retryReads);
  cfg.SetApplicationName(appNameStr0);
  cfg.SetLoginTimeoutSeconds(loginTimeoutSec);
  cfg.SetReplicaSet(replicaSetStr0);
  cfg.SetDefaultFetchSize(fetchSize);
}
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace ignite
