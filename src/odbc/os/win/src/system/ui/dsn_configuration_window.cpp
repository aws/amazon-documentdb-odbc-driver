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

#include <Windowsx.h>
#include <Shlwapi.h>

#include "ignite/odbc/log.h"
#include "ignite/odbc/read_preference.h"
#include "ignite/odbc/scan_method.h"

#include "ignite/odbc/system/ui/dsn_configuration_window.h"
#include "ignite/odbc/config/config_tools.h"

namespace ignite
{
    namespace odbc
    {
        namespace system
        {
            namespace ui
            {   
                DsnConfigurationWindow::DsnConfigurationWindow(Window* parent, config::Configuration& config):
                    CustomWindow(parent, "IgniteConfigureDsn", "Configure Amazon DocumentDB DSN Latest"),
                    width(730),
                    height(515),
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
                    // internal SSH tunnel vars
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
                    // end of SSH vars
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
                    created(false)
                {
                    // No-op.
                }

                DsnConfigurationWindow::~DsnConfigurationWindow()
                {
                    // No-op.
                }

                void DsnConfigurationWindow::Create()
                {
                    // Finding out parent position.
                    RECT parentRect;
                    GetWindowRect(parent->GetHandle(), &parentRect);

                    // Positioning window to the center of parent window.
                    const int posX = parentRect.left + (parentRect.right - parentRect.left - width) / 2;
                    const int posY = parentRect.top + (parentRect.bottom - parentRect.top - height) / 2;

                    RECT desiredRect = {posX, posY, posX + width, posY + height};
                    AdjustWindowRect(&desiredRect, WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, FALSE);

                    Window::Create(WS_OVERLAPPED | WS_SYSMENU, desiredRect.left, desiredRect.top,
                        desiredRect.right - desiredRect.left, desiredRect.bottom - desiredRect.top, 0);

                    if (!handle)
                    {
                        std::stringstream buf;

                        buf << "Can not create window, error code: " << GetLastError();

                        throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
                    }
                }
                
                void DsnConfigurationWindow::OnCreate()
                {
                    int groupPosYLeft = MARGIN;
                    int groupSizeY = width / 2 - 2 * MARGIN;
                    int posXRight = width / 2 + MARGIN;
                    int groupPosYRight = MARGIN;

                    // create left column group settings
                    groupPosYLeft += INTERVAL + CreateConnectionSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
                    groupPosYLeft += INTERVAL + CreateTlsSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
                    groupPosYLeft += INTERVAL + CreateSchemaSettingsGroup(MARGIN, groupPosYLeft, groupSizeY);
                    // create right column group settings 
                    groupPosYRight += INTERVAL + CreateSshSettingsGroup(posXRight, groupPosYRight, groupSizeY);
                    groupPosYRight += INTERVAL + CreateAdditionalSettingsGroup(posXRight, groupPosYRight, groupSizeY);

                    int cancelPosX = width - MARGIN - BUTTON_WIDTH;
                    int okPosX = cancelPosX - INTERVAL - BUTTON_WIDTH;

                    okButton = CreateButton(okPosX, groupPosYRight, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok", ChildId::OK_BUTTON);
                    cancelButton = CreateButton(cancelPosX, groupPosYRight, BUTTON_WIDTH, BUTTON_HEIGHT,
                        "Cancel", ChildId::CANCEL_BUTTON);

                    // check whether the required fields are filled. If not, Ok button is disabled.
                    created = true;
                    okButton->SetEnabled(
                        nameEdit->HasText() && userEdit->HasText() 
                        && passwordEdit->HasText() && databaseEdit->HasText() 
                        && hostnameEdit->HasText() && portEdit->HasText());
                }

                int DsnConfigurationWindow::CreateConnectionSettingsGroup(int posX, int posY, int sizeX)
                {
                    enum { LABEL_WIDTH = 100 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;

                    const char* val = config.GetDsn().c_str();
                    nameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Data Source Name*:", ChildId::NAME_LABEL);
                    nameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::NAME_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetHostname().c_str();
                    hostnameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, 
                        "Hostname*:", ChildId::HOST_NAME_LABEL);
                    hostnameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, 
                        val, ChildId::HOST_NAME_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    std::string tmp = common::LexicalCast<std::string>(config.GetTcpPort());
                    val = tmp.c_str();
                    portLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Port*:", ChildId::PORT_LABEL);
                    portEdit = CreateEdit(
                        editPosX, rowPos, editSizeX, ROW_HEIGHT,
                        val, ChildId::PORT_EDIT, ES_NUMBER);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetDatabase().c_str();
                    databaseLabel =
                        CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "Database*:", ChildId::DATABASE_LABEL);
                    databaseEdit = CreateEdit(editPosX, rowPos, editSizeX,
                                          ROW_HEIGHT, val, ChildId::DATABASE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetUser().c_str();
                    userLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, "User* :", ChildId::USER_LABEL);
                    userEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::USER_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetPassword().c_str();
                    passwordLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Password*:", ChildId::PASSWORD_LABEL);
                    passwordEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                        val, ChildId::USER_EDIT, ES_PASSWORD);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    connectionSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "Connection Settings", ChildId::CONNECTION_SETTINGS_GROUP_BOX);

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateSshSettingsGroup(int posX, int posY, int sizeX) 
                {
                    enum { LABEL_WIDTH = 120 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;

                    int checkBoxSize = sizeX - 2 * MARGIN;

                    sshEnableCheckBox = CreateCheckBox(
                        labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, "Enable SSH Tunnel",
                        ChildId::SSH_ENABLE_CHECK_BOX, config.IsSshEnable());
                   
                    rowPos += INTERVAL + ROW_HEIGHT;

                    const char* val = config.GetSshUser().c_str();
                    sshUserLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, "SSH User:", ChildId::SSH_USER_LABEL);
                    sshUserEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::SSH_USER_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;
                    
                    val = config.GetSshHost().c_str();
                    sshHostLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "SSH Hostname:", ChildId::SSH_HOST_LABEL);
                    sshHostEdit = CreateEdit(editPosX, rowPos, editSizeX,
                                          ROW_HEIGHT, val, ChildId::SSH_HOST_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetSshPrivateKeyFile().c_str();
                    sshPrivateKeyFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "SSH Private Key File:", ChildId::SSH_PRIVATE_KEY_FILE_LABEL);
                    sshPrivateKeyFileEdit = CreateEdit(editPosX, rowPos, editSizeX,
                                          ROW_HEIGHT, val, ChildId::SSH_PRIVATE_KEY_FILE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetSshPrivateKeyPassphrase().c_str();
                    // ssh private key passphrase label requires double the row height due to the long label.
                    sshPrivateKeyPassphraseLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT * 2,
                                    "SSH Private Key Passphrase:", ChildId::SSH_PRIVATE_KEY_PASSPHRASE_LABEL);
                    sshPrivateKeyPassphraseEdit = CreateEdit(editPosX, rowPos, editSizeX,
                                          ROW_HEIGHT * 2, val, ChildId::SSH_PRIVATE_KEY_PASSPHRASE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT * 2;

                    // SSH Strict Host Key Check check box needs to have editSizeX as size because the string is long
                    sshStrictHostKeyCheckingCheckBox = CreateCheckBox(
                        labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, "SSH Strict Host Key Check (disabling option is less secure)",
                        ChildId::SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX, config.IsSshStrictHostKeyChecking());

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetSshKnownHostsFile().c_str();
                    sshKnownHostsFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "SSH Known Hosts File:", ChildId::SSH_KNOWN_HOSTS_FILE_LABEL);
                    sshKnownHostsFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::SSH_KNOWN_HOSTS_FILE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    sshSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                                       "Internal SSH Tunnel Settings",
                                       ChildId::SSH_SETTINGS_GROUP_BOX);

                    sshUserEdit->SetEnabled(sshEnableCheckBox->IsChecked());
                    sshHostEdit->SetEnabled(sshEnableCheckBox->IsChecked());
                    sshPrivateKeyFileEdit->SetEnabled( sshEnableCheckBox->IsChecked());
                    sshPrivateKeyPassphraseEdit->SetEnabled(sshEnableCheckBox->IsChecked());
                    sshStrictHostKeyCheckingCheckBox->SetEnabled(sshEnableCheckBox->IsChecked());
                    sshKnownHostsFileEdit->SetEnabled(
                        sshEnableCheckBox->IsChecked()
                        && sshStrictHostKeyCheckingCheckBox->IsChecked());

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateTlsSettingsGroup(int posX, int posY, int sizeX)
                { 

                    enum { LABEL_WIDTH = 100 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;

                    int checkBoxSize = sizeX - 2 * MARGIN;

                    tlsCheckBox = CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                        "Enable TLS", ChildId::TLS_CHECK_BOX, config.IsTls());

                    rowPos += INTERVAL + ROW_HEIGHT;

                    tlsAllowInvalidHostnamesCheckBox = CreateCheckBox(
                        labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, "Allow Invalid Hostnames (enabling option is less secure)",
                        ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
                        config.IsTlsAllowInvalidHostnames());

                    rowPos += INTERVAL + ROW_HEIGHT;

                    const char* val = config.GetTlsCaFile().c_str();
                    tlsCaFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "TLS CA File:", ChildId::TLS_CA_FILE_LABEL);
                    tlsCaFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::TLS_CA_FILE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    tlsSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "TLS/SSL Settings", ChildId::TLS_SETTINGS_GROUP_BOX);

                    tlsAllowInvalidHostnamesCheckBox->SetEnabled(tlsCheckBox->IsChecked());
                    tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateSchemaSettingsGroup(int posX, int posY, int sizeX)
                {   

                    enum { LABEL_WIDTH = 100 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;
                    
                    int checkBoxSize = sizeX - 2 * MARGIN;

                    ScanMethod::Type scanMethod = config.GetScanMethod();

                    scanMethodLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Scan Method:", ChildId::SCAN_METHOD_LABEL);
                    scanMethodComboBox = CreateComboBox(
                        editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                       "", ChildId::SCAN_METHOD_COMBO_BOX);

		            scanMethodComboBox->AddString("Random");
                    scanMethodComboBox->AddString("ID Forward");
                    scanMethodComboBox->AddString("ID Reverse");
                    scanMethodComboBox->AddString("All");

                    scanMethodComboBox->SetSelection(scanMethod); // set default

                    rowPos += INTERVAL + ROW_HEIGHT;

                    std::string tmp = common::LexicalCast<std::string>(config.GetScanLimit());
                    const char* val = tmp.c_str();
                    scanLimitLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Scan Limit:", ChildId::SCAN_LIMIT_LABEL);
                    scanLimitEdit = CreateEdit(
                        editPosX, rowPos, editSizeX, ROW_HEIGHT,
                        val, ChildId::SCAN_LIMIT_EDIT, ES_NUMBER);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetDatabase().c_str();
                    schemaLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Schema Name:", ChildId::SCHEMA_LABEL);
                    schemaEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::SCHEMA_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    refreshSchemaCheckBox = CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                        "Refresh Schema", ChildId::REFRESH_SCHEMA_CHECK_BOX, config.IsRefreshSchema());

                    rowPos += INTERVAL + ROW_HEIGHT;

                    schemaSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "Schema Generation Settings", ChildId::SCHEMA_SETTINGS_GROUP_BOX);

                    std::string scanMethodStr;
                    scanMethodComboBox->GetText(scanMethodStr);
                    if (ScanMethod::FromString(scanMethodStr,
                                               ScanMethod::UNKNOWN)
                        == ScanMethod::ALL) {
                        scanLimitEdit->SetEnabled(false);
                    } else {
                        scanLimitEdit->SetEnabled(true);
                    }

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateAdditionalSettingsGroup(int posX, int posY, int sizeX) 
                {
                    enum { LABEL_WIDTH = 120 }; // same as SSH settings

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int checkBoxSize = (sizeX - 3 * INTERVAL) / 2;

                    int rowPos = posY + 2 * INTERVAL;

                    retryReadsCheckBox = CreateCheckBox(
                        labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                        "Retry Reads", ChildId::RETRY_READS_CHECK_BOX,
                        config.IsRetryReads());

                    rowPos += INTERVAL + ROW_HEIGHT;

                    ReadPreference::Type readPreference = config.GetReadPreference();

                    readPreferenceLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Read preference:", ChildId::READ_PREFERENCE_LABEL);
                    readPreferenceComboBox = CreateComboBox(
                        editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                       "", ChildId::READ_PREFERENCE_COMBO_BOX);

		            readPreferenceComboBox->AddString("Primary");
                    readPreferenceComboBox->AddString("Primary Preferred");
                    readPreferenceComboBox->AddString("Secondary");
                    readPreferenceComboBox->AddString("Secondary Preferred");
                    readPreferenceComboBox->AddString("Nearest");

                    readPreferenceComboBox->SetSelection(readPreference); // set default

                    rowPos += INTERVAL + ROW_HEIGHT;
                    
                    const char* val = config.GetApplicationName().c_str();
                    appNameLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Application Name:", ChildId::APP_NAME_LABEL);
                    appNameEdit =
                        CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::APP_NAME_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    std::string tmp = common::LexicalCast< std::string >(
                        config.GetLoginTimeoutSeconds());
                    val = tmp.c_str();
                    loginTimeoutSecLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Login Timeout (s):", ChildId::LOGIN_TIMEOUT_SEC_LABEL);

                    loginTimeoutSecEdit =
                        CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::LOGIN_TIMEOUT_SEC_EDIT, ES_NUMBER);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetReplicaSet().c_str();
                    replicaSetLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "Replica Set:", ChildId::REPLICA_SET_LABEL);
                    replicaSetEdit =
                        CreateEdit(editPosX, rowPos, editSizeX,
                                   ROW_HEIGHT, val,
                                   ChildId::REPLICA_SET_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    tmp = common::LexicalCast< std::string >(
                        config.GetDefaultFetchSize());
                    val = tmp.c_str();
                    defaultFetchSizeLabel = CreateLabel(
                        labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Fetch Size:", ChildId::DEFAULT_FETCH_SIZE_LABEL);

                    defaultFetchSizeEdit = CreateEdit(
                        editPosX, rowPos, editSizeX, ROW_HEIGHT,
                        val, ChildId::DEFAULT_FETCH_SIZE_EDIT, ES_NUMBER);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    additionalSettingsGroupBox = CreateGroupBox(posX, posY, sizeX,
                                       rowPos - posY, "Additional Settings",
                        ChildId::ADDITIONAL_SETTINGS_GROUP_BOX);

                    return rowPos - posY;
                }

                bool DsnConfigurationWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
                {
                    switch (msg)
                    {
                        case WM_COMMAND:
                        {
                            switch (LOWORD(wParam))
                            {
                                case ChildId::OK_BUTTON:
                                {
                                    try
                                    {
                                        RetrieveParameters(config);

                                        accepted = true;

                                        PostMessage(GetHandle(), WM_CLOSE, 0, 0);
                                    }
                                    catch (IgniteError& err)
                                    {
                                        MessageBox(NULL, err.GetText(), "Error!", MB_ICONEXCLAMATION | MB_OK);
                                    }

                                    break;
                                }

                                case IDCANCEL:
                                case ChildId::CANCEL_BUTTON:
                                {
                                    PostMessage(GetHandle(), WM_CLOSE, 0, 0);

                                    break;
                                }

                                case ChildId::NAME_EDIT:
                                case ChildId::HOST_NAME_EDIT:
                                case ChildId::PORT_EDIT:
                                case ChildId::DATABASE_EDIT:
                                case ChildId::USER_EDIT:
                                case ChildId::PASSWORD_EDIT:
                                { 
                                    // if window has been created. Check
                                    if (created) {
                                        okButton->SetEnabled(
                                            nameEdit->HasText()
                                            && userEdit->HasText()
                                            && passwordEdit->HasText()
                                            && databaseEdit->HasText()
                                            && hostnameEdit->HasText()
                                            && portEdit->HasText());                                 
                                    }
                                    break;
                                }


                                case ChildId::SSH_ENABLE_CHECK_BOX: 
                                {
                                    sshEnableCheckBox->SetChecked(!sshEnableCheckBox->IsChecked());
                                    sshUserEdit->SetEnabled(
                                        sshEnableCheckBox->IsChecked());
                                    sshHostEdit->SetEnabled(sshEnableCheckBox->IsChecked());
                                    sshPrivateKeyFileEdit->SetEnabled(
                                        sshEnableCheckBox->IsChecked());
                                    sshPrivateKeyPassphraseEdit->SetEnabled(
                                        sshEnableCheckBox->IsChecked());
                                    sshStrictHostKeyCheckingCheckBox
                                        ->SetEnabled(
                                            sshEnableCheckBox->IsChecked());
                                    sshKnownHostsFileEdit->SetEnabled(
                                        sshEnableCheckBox->IsChecked() 
                                        && sshStrictHostKeyCheckingCheckBox->IsChecked());

                                    break;
                                }

                                case ChildId::SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX:
                                {
                                    sshStrictHostKeyCheckingCheckBox
                                        ->SetChecked(!sshStrictHostKeyCheckingCheckBox->IsChecked());
                                    sshKnownHostsFileEdit->SetEnabled(
                                        sshEnableCheckBox->IsChecked()
                                        && sshStrictHostKeyCheckingCheckBox->IsChecked());
                                    break;
                                }

                                case ChildId::TLS_CHECK_BOX: 
                                {
                                    tlsCheckBox->SetChecked(!tlsCheckBox->IsChecked());
                                    tlsAllowInvalidHostnamesCheckBox
                                        ->SetEnabled(tlsCheckBox->IsChecked());
                                    tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());

                                    break;
                                }


                                case ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX:
                                {
                                    tlsAllowInvalidHostnamesCheckBox->SetChecked(!tlsAllowInvalidHostnamesCheckBox->IsChecked());

                                    break;
                                }

                                case ChildId::SCAN_METHOD_COMBO_BOX:
                                {
                                    std::string scanMethodStr;
                                    scanMethodComboBox->GetText(scanMethodStr);
                                    if (ScanMethod::FromString(
                                                scanMethodStr, ScanMethod::UNKNOWN)
                                        == ScanMethod::ALL) 
                                    {
                                        scanLimitEdit->SetEnabled(false);
                                    } 
                                    else 
                                    {
                                        scanLimitEdit->SetEnabled(true);
                                    }
                                    break;
                                }

                                case ChildId::REFRESH_SCHEMA_CHECK_BOX:
                                {
                                    refreshSchemaCheckBox->SetChecked(!refreshSchemaCheckBox->IsChecked());
                                    break;
                                }

                                case ChildId::RETRY_READS_CHECK_BOX:
                                {
                                    retryReadsCheckBox->SetChecked(!retryReadsCheckBox->IsChecked());

                                    break;
                                }

                                default:
                                    return false;
                            }

                            break;
                        }

                        case WM_DESTROY:
                        {
                            PostQuitMessage(accepted ? Result::OK : Result::CANCEL);

                            break;
                        }

                        default:
                            return false;
                    }

                    return true;
                }

                void DsnConfigurationWindow::RetrieveParameters(config::Configuration& cfg) const
                {
                    RetrieveConnectionParameters(cfg);
                    RetrieveSshParameters(cfg);
                    RetrieveTlsParameters(cfg);
                    RetrieveSchemaParameters(cfg);
                    RetrieveAdditionalParameters(cfg);
                }

                void DsnConfigurationWindow::RetrieveConnectionParameters(config::Configuration& cfg) const
                {
                    std::string dsnStr;
                    std::string hostnameStr;
                    std::string portStr;
                    std::string databaseStr;
                    std::string userStr;
                    std::string passwordStr;

                    nameEdit->GetText(dsnStr);

                    common::StripSurroundingWhitespaces(dsnStr);
                    // Stripping of whitespaces off the schema skipped intentionally

                    hostnameEdit->GetText(hostnameStr);
                    portEdit->GetText(portStr);
                    databaseEdit->GetText(databaseStr);
                    userEdit->GetText(userStr);
                    passwordEdit->GetText(passwordStr);

                    int16_t port = common::LexicalCast< int16_t >(portStr);

                    if (port <= 0)
                        port = config.GetTcpPort();

                    LOG_MSG("Retrieving arguments:");
                    LOG_MSG("DSN:      " << dsnStr);
                    LOG_MSG("Hostname: " << hostnameStr);
                    LOG_MSG("Port:     " << portStr);
                    LOG_MSG("Database: " << databaseStr);

                    // username and password intentionally not logged for security reasons

                    cfg.SetDsn(dsnStr);
                    cfg.SetTcpPort(port);
                    cfg.SetHostname(hostnameStr);
                    cfg.SetDatabase(databaseStr);
                    cfg.SetUser(userStr);
                    cfg.SetPassword(passwordStr);
                }

                void DsnConfigurationWindow::RetrieveSshParameters(config::Configuration& cfg) const
                {
                    bool sshEnable = sshEnableCheckBox->IsChecked();
                    bool sshStrictHostKeyChecking = sshStrictHostKeyCheckingCheckBox->IsChecked();

                    std::string sshUserStr;
                    std::string sshHostStr;
                    std::string sshPrivateKeyFileStr;
                    std::string sshPrivateKeyPassphraseStr;
                    std::string sshKnownHostsFileStr;

                    sshUserEdit->GetText(sshUserStr);
                    sshHostEdit->GetText(sshHostStr);
                    sshPrivateKeyFileEdit->GetText(sshPrivateKeyFileStr);
                    sshPrivateKeyPassphraseEdit->GetText(sshPrivateKeyPassphraseStr);
                    sshKnownHostsFileEdit->GetText(sshKnownHostsFileStr);

                    LOG_MSG("Retrieving arguments:");
                    LOG_MSG("SSH enable:                    " << (sshEnable ? "true" : "false"));
                    LOG_MSG("SSH user:                      " << sshUserStr);
                    LOG_MSG("SSH host:                      " << sshHostStr);
                    LOG_MSG("SSH private key file:          " << sshPrivateKeyFileStr);
                    LOG_MSG("SSH private key passphrase:    " << sshPrivateKeyPassphraseStr);
                    LOG_MSG("SSH strict host key checking:  " << (sshStrictHostKeyChecking ? "true" : "false"));
                    LOG_MSG("SSH known hosts file:          " << sshKnownHostsFileStr);

                    cfg.SetSshEnable(sshEnable);
                    cfg.SetSshUser(sshUserStr);
                    cfg.SetSshHost(sshHostStr);
                    cfg.SetSshPrivateKeyFile(sshPrivateKeyFileStr);
                    cfg.SetSshPrivateKeyPassphrase(sshPrivateKeyPassphraseStr);
                    cfg.SetSshStrictHostKeyChecking(sshStrictHostKeyChecking);
                    cfg.SetSshKnownHostsFile(sshKnownHostsFileStr);
                }

                void DsnConfigurationWindow::RetrieveTlsParameters(config::Configuration& cfg) const
                {
                    bool tls = tlsCheckBox->IsChecked();
                    bool tlsAllowInvalidHostnames = tlsAllowInvalidHostnamesCheckBox->IsChecked();
                    std::string tlsCaStr;

                    tlsCaFileEdit->GetText(tlsCaStr);

                    LOG_MSG("TLS/SSL Encryption:                       " << (tls ? "true" : "false"));
                    LOG_MSG("tls Allow Invalid Hostnames:              " << (tlsAllowInvalidHostnames ? "true" : "false"));
                    LOG_MSG("TLS CA (Certificate Authority) File name: " << tlsCaStr);

                    cfg.SetTls(tls);
                    cfg.SetTlsAllowInvalidHostnames(tlsAllowInvalidHostnames);
                    cfg.SetTlsCaFile(tlsCaStr);
                }

                void DsnConfigurationWindow::RetrieveSchemaParameters(config::Configuration& cfg) const
                {
                    std::string scanMethodStr;
                    std::string scanLimitStr;
                    std::string schemaStr;
                    bool refreshSchema = refreshSchemaCheckBox->IsChecked();

                    scanMethodComboBox->GetText(scanMethodStr);
                    scanLimitEdit->GetText(scanLimitStr);
                    schemaEdit->GetText(schemaStr);

                    int32_t scanLimit =
                        common::LexicalCast<int32_t>(scanLimitStr);

                    if (scanLimit <= 0)
                        scanLimit = config.GetScanLimit();
                    
                    LOG_MSG("Scan method:    " << scanMethodStr);
                    LOG_MSG("Scan limit      " << scanLimit);
                    LOG_MSG("Schema:         " << schemaStr);
                    LOG_MSG("Refresh schema: " << (refreshSchema ? "true" : "false"));

                    ScanMethod::Type scanMethod =
                        ScanMethod::FromString(scanMethodStr, ScanMethod::UNKNOWN);

                    cfg.SetScanMethod(scanMethod);
                    cfg.SetSchemaName(schemaStr);
                    cfg.SetScanLimit(scanLimit);
                    cfg.SetRefreshSchema(refreshSchema);
                }

                void DsnConfigurationWindow::RetrieveAdditionalParameters(config::Configuration& cfg) const
                {
                    std::string readPreferenceStr;
                    std::string appNameStr;
                    std::string replicaSetStr;

                    readPreferenceComboBox->GetText(readPreferenceStr);
                    appNameEdit->GetText(appNameStr);
                    replicaSetEdit->GetText(replicaSetStr);

                    bool retryReads = retryReadsCheckBox->IsChecked();

                    std::string loginTimeoutSecStr;

                    loginTimeoutSecEdit->GetText(loginTimeoutSecStr);

                    int32_t loginTimeoutSec =
                        common::LexicalCast<int32_t>(loginTimeoutSecStr);

                    if (loginTimeoutSec <= 0)
                        loginTimeoutSec = config.GetLoginTimeoutSeconds();

                    std::string fetchSizeStr;

                    defaultFetchSizeEdit->GetText(fetchSizeStr);

                    int32_t fetchSize =
                        common::LexicalCast<int32_t>(fetchSizeStr);

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
            }
        }
    }
}
