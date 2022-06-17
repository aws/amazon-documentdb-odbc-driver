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

  std::wstring wVal = utility::FromUtf8(config.GetDsn());
  nameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                          L"Data Source Name*:", ChildId::NAME_LABEL);
  nameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetHostname());
  hostnameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Hostname*:", ChildId::HOST_NAME_LABEL);
  hostnameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                            ChildId::HOST_NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetPort());
  portLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"Port*:",
                          ChildId::PORT_LABEL);
  portEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::PORT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetDatabase());
  databaseLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Database*:", ChildId::DATABASE_LABEL);
  databaseEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                            ChildId::DATABASE_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetUser());
  userLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                          L"User* :", ChildId::USER_LABEL);
  userEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::USER_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetPassword());
  passwordLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Password*:", ChildId::PASSWORD_LABEL);
  passwordEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
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

  std::wstring wVal = utility::FromUtf8(config.GetSshUser());
  sshUserLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"SSH User:", ChildId::SSH_USER_LABEL);
  sshUserEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                           ChildId::SSH_USER_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSshHost());
  sshHostLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"SSH Hostname:", ChildId::SSH_HOST_LABEL);
  sshHostEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                           ChildId::SSH_HOST_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSshPrivateKeyFile());
  sshPrivateKeyFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                                       ROW_HEIGHT, L"SSH Private Key File:",
                                       ChildId::SSH_PRIVATE_KEY_FILE_LABEL);
  sshPrivateKeyFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                     wVal, ChildId::SSH_PRIVATE_KEY_FILE_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSshPrivateKeyPassphrase());
  // ssh private key passphrase label requires double the row height due to the
  // long label.
  sshPrivateKeyPassphraseLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT * 2,
                  L"SSH Private Key File Passphrase:",
                  ChildId::SSH_PRIVATE_KEY_PASSPHRASE_LABEL);
  sshPrivateKeyPassphraseEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
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

  wVal = utility::FromUtf8(config.GetSshKnownHostsFile());
  sshKnownHostsFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                                       ROW_HEIGHT, L"SSH Known Hosts File:",
                                       ChildId::SSH_KNOWN_HOSTS_FILE_LABEL);
  sshKnownHostsFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                     wVal, ChildId::SSH_KNOWN_HOSTS_FILE_EDIT);

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
                                    L"", ChildId::LOG_LEVEL_COMBO_BOX);

  logLevelComboBox->AddString(L"Debug");
  logLevelComboBox->AddString(L"Info");
  logLevelComboBox->AddString(L"Error");
  logLevelComboBox->AddString(L"Off");

  logLevelComboBox->SetSelection(static_cast< int >(logLevel));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetLogPath());
  logPathLabel = CreateLabel(
      labelPosX, rowPos, pathSizeX, ROW_HEIGHT * 2,
      L"Log Path:\n(the log file name format is docdb_odbc_YYYYMMDD.log)",
      ChildId::LOG_PATH_LABEL);

  rowPos += INTERVAL * 2 + ROW_HEIGHT;

  logPathEdit = CreateEdit(editPosX, rowPos, pathSizeX, ROW_HEIGHT, wVal,
                           ChildId::LOG_PATH_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  logSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"Log Settings",
                     ChildId::LOG_SETTINGS_GROUP_BOX);

  std::wstring logLevelWStr;
  logLevelComboBox->GetText(logLevelWStr);
  if (LogLevel::FromString(utility::ToUtf8(logLevelWStr),
                           LogLevel::Type::UNKNOWN)
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

  tlsAllowInvalidHostnamesCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
      L"Allow Invalid Hostnames (enabling option is less secure)",
      ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
      config.IsTlsAllowInvalidHostnames());

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetTlsCaFile());
  tlsCaFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                               L"TLS CA File:", ChildId::TLS_CA_FILE_LABEL);
  tlsCaFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
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

  std::wstring wVal = std::to_wstring(config.GetScanLimit());
  scanLimitLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                               L"Scan Limit:", ChildId::SCAN_LIMIT_LABEL);
  scanLimitEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                             ChildId::SCAN_LIMIT_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSchemaName());
  schemaLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                            L"Schema Name:", ChildId::SCHEMA_LABEL);
  schemaEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
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

  std::wstring scanMethodWStr;
  scanMethodComboBox->GetText(scanMethodWStr);
  if (ScanMethod::FromString(utility::ToUtf8(scanMethodWStr),
                             ScanMethod::Type::UNKNOWN)
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

  std::wstring wVal = utility::FromUtf8(config.GetApplicationName());
  appNameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"Application Name:", ChildId::APP_NAME_LABEL);
  appNameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                           ChildId::APP_NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetLoginTimeoutSeconds());
  loginTimeoutSecLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Login Timeout (s):", ChildId::LOGIN_TIMEOUT_SEC_LABEL);

  loginTimeoutSecEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                 ChildId::LOGIN_TIMEOUT_SEC_EDIT, ES_NUMBER);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetReplicaSet());
  replicaSetLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                L"Replica Set:", ChildId::REPLICA_SET_LABEL);
  replicaSetEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                              ChildId::REPLICA_SET_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetDefaultFetchSize());
  defaultFetchSizeLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"Fetch Size:",
                  ChildId::DEFAULT_FETCH_SIZE_LABEL);

  defaultFetchSizeEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
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
            std::wstring errWText = utility::FromUtf8(err.GetText());
            MessageBox(NULL, errWText.c_str(), L"Error!",
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
          std::wstring logLevelWStr;
          logLevelComboBox->GetText(logLevelWStr);
          if (LogLevel::FromString(utility::ToUtf8(logLevelWStr),
                                   LogLevel::Type::UNKNOWN)
              == LogLevel::Type::OFF) {
            logPathEdit->SetEnabled(false);
          } else {
            logPathEdit->SetEnabled(true);
          }
          break;
        }

        case ChildId::SCAN_METHOD_COMBO_BOX: {
          std::wstring scanMethodWStr;
          scanMethodComboBox->GetText(scanMethodWStr);
          if (ScanMethod::FromString(utility::ToUtf8(scanMethodWStr),
                                     ScanMethod::Type::UNKNOWN)
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
  std::wstring dsnWStr;
  std::wstring hostnameWStr;
  std::wstring portWStr;
  std::wstring databaseWStr;
  std::wstring userWStr;
  std::wstring passwordWStr;

  nameEdit->GetText(dsnWStr);
  std::string dsnStr = utility::ToUtf8(dsnWStr);
  common::StripSurroundingWhitespaces(dsnStr);
  // Stripping of whitespaces off the schema skipped intentionally

  hostnameEdit->GetText(hostnameWStr);
  portEdit->GetText(portWStr);
  databaseEdit->GetText(databaseWStr);
  userEdit->GetText(userWStr);
  passwordEdit->GetText(passwordWStr);

  std::string hostnameStr = utility::ToUtf8(hostnameWStr);
  std::string portStr = utility::ToUtf8(portWStr);
  std::string databaseStr = utility::ToUtf8(databaseWStr);
  std::string userStr = utility::ToUtf8(userWStr);
  std::string passwordStr = utility::ToUtf8(passwordWStr);

  int16_t port = common::LexicalCast< int16_t >(portStr);

  if (port <= 0)
    port = config.GetPort();

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("DSN:      " << dsnStr);
  LOG_MSG("Hostname: " << hostnameStr);
  LOG_MSG("Port:     " << portStr);
  LOG_MSG("Database: " << databaseStr);

  // username and password intentionally not logged for security reasons

  cfg.SetDsn(dsnStr);
  cfg.SetPort(port);
  cfg.SetHostname(hostnameStr);
  cfg.SetDatabase(databaseStr);
  cfg.SetUser(userStr);
  cfg.SetPassword(passwordStr);
}

void DsnConfigurationWindow::RetrieveSshParameters(
    config::Configuration& cfg) const {
  bool sshEnable = sshEnableCheckBox->IsChecked();
  bool sshStrictHostKeyChecking = sshStrictHostKeyCheckingCheckBox->IsChecked();

  std::wstring sshUserWStr;
  std::wstring sshHostWStr;
  std::wstring sshPrivateKeyFileWStr;
  std::wstring sshPrivateKeyPassphraseWStr;
  std::wstring sshKnownHostsFileWStr;

  sshUserEdit->GetText(sshUserWStr);
  sshHostEdit->GetText(sshHostWStr);
  sshPrivateKeyFileEdit->GetText(sshPrivateKeyFileWStr);
  sshPrivateKeyPassphraseEdit->GetText(sshPrivateKeyPassphraseWStr);
  sshKnownHostsFileEdit->GetText(sshKnownHostsFileWStr);

  std::string sshUserStr = utility::ToUtf8(sshUserWStr);
  std::string sshHostStr = utility::ToUtf8(sshHostWStr);
  std::string sshPrivateKeyFileStr = utility::ToUtf8(sshPrivateKeyFileWStr);
  std::string sshPrivateKeyPassphraseStr =
      utility::ToUtf8(sshPrivateKeyPassphraseWStr);
  std::string sshKnownHostsFileStr = utility::ToUtf8(sshKnownHostsFileWStr);

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("SSH enable:                    " << (sshEnable ? "true" : "false"));
  LOG_MSG("SSH user:                      " << sshUserStr);
  LOG_MSG("SSH host:                      " << sshHostStr);
  LOG_MSG("SSH private key file:          " << sshPrivateKeyFileStr);
  LOG_MSG("SSH strict host key checking:  "
          << (sshStrictHostKeyChecking ? "true" : "false"));
  LOG_MSG("SSH known hosts file:          " << sshKnownHostsFileStr);

  cfg.SetSshEnable(sshEnable);
  cfg.SetSshUser(sshUserStr);
  cfg.SetSshHost(sshHostStr);
  cfg.SetSshPrivateKeyFile(sshPrivateKeyFileStr);
  cfg.SetSshPrivateKeyPassphrase(sshPrivateKeyPassphraseStr);
  cfg.SetSshStrictHostKeyChecking(sshStrictHostKeyChecking);
  cfg.SetSshKnownHostsFile(sshKnownHostsFileStr);
}

// RetrieveLogParameters is a special case. We want to get the log level and
// path as soon as possible. If user set log level to OFF, then nothing should
// be logged. Therefore, the LOG_MSG calls are after log level and log path are
// set.
void DsnConfigurationWindow::RetrieveLogParameters(
    config::Configuration& cfg) const {
  std::wstring logLevelWStr;
  std::wstring logPathWStr;

  logLevelComboBox->GetText(logLevelWStr);
  logPathEdit->GetText(logPathWStr);

  std::string logLevelStr = utility::ToUtf8(logLevelWStr);
  std::string logPathStr = utility::ToUtf8(logPathWStr);

  LogLevel::Type logLevel =
      LogLevel::FromString(logLevelStr, LogLevel::Type::UNKNOWN);

  cfg.SetLogLevel(logLevel);
  cfg.SetLogPath(logPathStr);

  LOG_MSG("Log level:    " << logLevelStr);
  LOG_MSG("Log path:     " << logPathStr);
}

void DsnConfigurationWindow::RetrieveTlsParameters(
    config::Configuration& cfg) const {
  bool tls = tlsCheckBox->IsChecked();
  bool tlsAllowInvalidHostnames = tlsAllowInvalidHostnamesCheckBox->IsChecked();
  std::wstring tlsCaWStr;

  tlsCaFileEdit->GetText(tlsCaWStr);

  std::string tlsCaStr = utility::ToUtf8(tlsCaWStr);

  LOG_MSG(
      "TLS/SSL Encryption:                       " << (tls ? "true" : "false"));
  LOG_MSG("TLS Allow Invalid Hostnames:              "
          << (tlsAllowInvalidHostnames ? "true" : "false"));
  LOG_MSG("TLS CA (Certificate Authority) File: " << tlsCaStr);

  cfg.SetTls(tls);
  cfg.SetTlsAllowInvalidHostnames(tlsAllowInvalidHostnames);
  cfg.SetTlsCaFile(tlsCaStr);
}

void DsnConfigurationWindow::RetrieveSchemaParameters(
    config::Configuration& cfg) const {
  std::wstring scanMethodWStr;
  std::wstring scanLimitWStr;
  std::wstring schemaWStr;
  bool refreshSchema = refreshSchemaCheckBox->IsChecked();

  scanMethodComboBox->GetText(scanMethodWStr);
  scanLimitEdit->GetText(scanLimitWStr);
  schemaEdit->GetText(schemaWStr);

  std::string scanMethodStr = utility::ToUtf8(scanMethodWStr);
  std::string scanLimitStr = utility::ToUtf8(scanLimitWStr);
  std::string schemaStr = utility::ToUtf8(schemaWStr);

  int32_t scanLimit = common::LexicalCast< int32_t >(scanLimitStr);

  if (scanLimit <= 0)
    scanLimit = config.GetScanLimit();

  LOG_MSG("Scan method:    " << scanMethodStr);
  LOG_MSG("Scan limit      " << scanLimit);
  LOG_MSG("Schema:         " << schemaStr);
  LOG_MSG("Refresh schema: " << (refreshSchema ? "true" : "false"));

  ScanMethod::Type scanMethod =
      ScanMethod::FromString(scanMethodStr, ScanMethod::Type::UNKNOWN);

  cfg.SetScanMethod(scanMethod);
  cfg.SetSchemaName(schemaStr);
  cfg.SetScanLimit(scanLimit);
  cfg.SetRefreshSchema(refreshSchema);
}

void DsnConfigurationWindow::RetrieveAdditionalParameters(
    config::Configuration& cfg) const {
  std::wstring readPreferenceWStr;
  std::wstring appNameWStr;
  std::wstring replicaSetWStr;
  std::wstring loginTimeoutSecWStr;
  std::wstring fetchSizeWStr;

  readPreferenceComboBox->GetText(readPreferenceWStr);
  appNameEdit->GetText(appNameWStr);
  replicaSetEdit->GetText(replicaSetWStr);
  bool retryReads = retryReadsCheckBox->IsChecked();
  loginTimeoutSecEdit->GetText(loginTimeoutSecWStr);
  defaultFetchSizeEdit->GetText(fetchSizeWStr);

  std::string readPreferenceStr = utility::ToUtf8(readPreferenceWStr);
  std::string appNameStr = utility::ToUtf8(appNameWStr);
  std::string replicaSetStr = utility::ToUtf8(replicaSetWStr);
  std::string loginTimeoutSecStr = utility::ToUtf8(loginTimeoutSecWStr);
  std::string fetchSizeStr = utility::ToUtf8(fetchSizeWStr);

  int32_t loginTimeoutSec = common::LexicalCast< int32_t >(loginTimeoutSecStr);
  if (loginTimeoutSec <= 0)
    loginTimeoutSec = config.GetLoginTimeoutSeconds();

  int32_t fetchSize = common::LexicalCast< int32_t >(fetchSizeStr);
  if (fetchSize <= 0)
    fetchSize = config.GetDefaultFetchSize();

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("Retry reads:              " << (retryReads ? "true" : "false"));
  LOG_MSG("Read preference:          " << readPreferenceStr);
  LOG_MSG("App name:                 " << appNameStr);
  LOG_MSG("Login timeout (seconds):  " << loginTimeoutSecStr);
  LOG_MSG("Replica Set:              " << replicaSetStr);
  LOG_MSG("Fetch size:               " << fetchSize);

  ReadPreference::Type readPreference = ReadPreference::FromString(
      readPreferenceStr, ReadPreference::Type::UNKNOWN);

  cfg.SetReadPreference(readPreference);
  cfg.SetRetryReads(retryReads);
  cfg.SetApplicationName(appNameStr);
  cfg.SetLoginTimeoutSeconds(loginTimeoutSec);
  cfg.SetReplicaSet(replicaSetStr);
  cfg.SetDefaultFetchSize(fetchSize);
}
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace ignite
