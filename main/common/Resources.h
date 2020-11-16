/*
    CaPriCe for Palm OS - Amstrad CPC 464/664/6128 emulator for Palm devices
    Copyright (C) 2009  Fr�d�ric Coste

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//
//  Resource: Alerts
//
#ifndef __WIN32__

#define NoHDScreenAlert                           1000
#define NoColorSupportAlert                       1001
#define NoOffscreenWindowsAlert                   1002
#define RomIncompatibleAlert                      1003
#define NotEnoughMemoryAlert                      1004
#define LargePNOCorruptedAlert                    1005
#define UnknownDeviceIDAlert                      1006
#define InvalidImageFormatAlert                   1007
#define BadWriteImageAlert                        1008
#define FilenameTooLongAlert                      1009
#define FormatDriveAlert                          1010
#define ARMProcessorAlert                         1011
#define UnableToRegisterAlert                     1012
#define WriteProtectedDiskAlert                   1013
#define BadCPCRestoreAlert                        1014
#define RestorationRequestAlert                   1015
#define WrongCaPriceRestoreAlert                  1017
#define BadCPCSaveAlert                           1018
#define CPCSaveCreationAlert                      1019
#define ConfirmResetAlert                         1021
#define ConfirmDeleteSavedSessionFileAlert        1022
#define ConfirmDeleteDiskFileAlert                1024
#define BadDeleteDiskFileAlert                    1025
#define NoVirtualFileSystemAlert                  1026
#define BadCreateDiskAlert                        1027
#define ScreenshotCreationAlert                   1028
#define BadScreenshotSaveAlert                    1029
#define ConfirmReplaceFileAlert                   1031
//
#define TraceAlert                                2000
#define CustomTraceAlert                          2001

#endif /* __WIN32__ */


//
//  Resource: Bitmaps
//
#define EmuKey_Reset_Released                     1000
#define EmuKey_Reset_Pushed                       1001
#define EmuKey_Pause_Released                     1002
#define EmuKey_Pause_Pushed                       1003
#define EmuKey_Joystick_Released                  1004
#define EmuKey_Joystick_Pushed                    1005
#define EmuKey_Fx_Released                        1006
#define EmuKey_Fx_Pushed                          1007
#define EmuKey_TrueSpeed_Released                 1008
#define EmuKey_TrueSpeed_Pushed                   1009
#define EmuKey_EmuSpeed_Released                  1010
#define EmuKey_EmuSpeed_Pushed                    1011
#define EmuKey_Sound_Released                     1012
#define EmuKey_Sound_Pushed                       1013
#define EmuKey_SubPanel_Released                  1014
#define EmuKey_SubPanel_Pushed                    1015
#define EmuKey_Sound_Volume_Released              1016
#define EmuKey_Sound_Volume_Pushed                1017
#define EmuKey_Background                         1099
#define CPCKey_ESC_Released                       1100
#define CPCKey_ESC_Pushed                         1101
#define CPCKey_COPY_Released                      1102
#define CPCKey_COPY_Pushed                        1103
#define CPCKey_CTRL_Released                      1104
#define CPCKey_CTRL_Pushed                        1105
#define CPCKey_SHIFT_Released                     1106
#define CPCKey_SHIFT_Pushed                       1107
#define CPCKey_CAPSLOCK_Released                  1108
#define CPCKey_CAPSLOCK_Pushed                    1109
#define CPCKey_TAB_Released                       1110
#define CPCKey_TAB_Pushed                         1111
#define CPCKey_DEL_Released                       1112
#define CPCKey_DEL_Pushed                         1113
#define CPCKey_ENTER_Released                     1114
#define CPCKey_ENTER_Pushed                       1115
#define CPCKey_CLR_Released                       1116
#define CPCKey_CLR_Pushed                         1117
#define CPCKey_AutoToggle_Released                1122
#define CPCKey_AutoToggle_Pushed                  1123
#define CPCKey_Background_1                       1198
#define CPCKey_Background_2                       1199
#define CPCKey_464_RSHIFT_Pushed                  1200
#define CPCKey_464_LSHIFT_Pushed                  1201
#define CPCKey_464_CTRL_Pushed                    1202
#define CPCKey_6128_RSHIFT_Pushed                 1203
#define CPCKey_6128_LSHIFT_Pushed                 CPCKey_464_RSHIFT_Pushed
#define CPCKey_6128_CTRL_Pushed                   CPCKey_464_RSHIFT_Pushed
#define ButtonsAreaBitmap                         1900
#define SubButtonsAreaBitmap                      1950
#define DrivesAreaBitmap                          2000
#define DriveKey_Swap_Released                    2001
#define DriveKey_Swap_Pushed                      2002
#define FDC_Led_OFF                               2003
#define FDC_Led_ON                                2004
#define DriveKey_Eject_Released                   2005
#define DriveKey_Eject_Pushed                     2006
#define Disk_Active                               2007
#define Shift_Key_Pressed                         2008
#define Ctrl_Key_Pressed                          2009
#define AutoToggleSpeed_First                     2100
#define SoundVolumeAdjust                         2150
#define Keyboard_464                              2500
#define Keyboard_664                              2501
#define Keyboard_6128                             2502
#define Mini_Keyboard                             2503


//
//  Resource: Forms
//
#ifndef __WIN32__
#define MainForm                                  1000
#define DriveForm                                 1100
#define AboutForm                                 1200
#define DeviceSettingsForm                        1300
#define EmulatorSettingsForm                      1400
#define SaveForm                                  1500
#define LoadForm                                  1600
#define ScreenOffsetForm                          1700
#define HardKeysForm                              1900
#endif /* __WIN32__ */


//
//  Resources: Menu
//
#ifndef __WIN32__
// File
#define FileLoadSessionMenu                       1100
#define FileSaveSessionMenu                       1101
#define FileLoadEjectDiskMenu                     1103
#define FileCreateDiskMenu                        1104
#define FileDeleteDiskMenu                        1105
#define FileSwapDiskMenu                          1106
#define FileScreenshotMenu                        1108
#define FileExitMenu                              1110
// Play
#define PlayPauseMenu                             1200
#define PlayResetMenu                             1201
#define PlayToggleJoystickMenu                    1203
#define PlayToggleSoundMenu                       1204
#define PlayToggleDecathlonMenu                   1205
#define PlayAutostartMenu                         1207
// Display
#define DisplayMiniKeyboardMenu                   1300
#define DisplayCPCDrivesMenu                      1301
#define DisplayCPCColouredKeyboardMenu            1302
#define DisplayCPCSpecialKeyboardMenu             1303
#define DisplayFullscreenMenu                     1305
#define DisplayOffsetMenu                         1306
#define DisplayOnScreenRockerHelpMenu             1308
#define DisplayKeycodesMenu                       1309
// Settings
#define SettingsDeviceMenu                        1400
#define SettingsEmulatorMenu                      1401
#define SettingsAboutMenu                         1403
#define SettingsHardKeysMenu                      1404
#define SettingsNightModeMenu                     1406
#endif /* __WIN32__ */


//
//  Resources: MainForm
//
#ifndef __WIN32__
// Custom Data
#define MainFormCustomData                        5000
//
#endif /* __WIN32__ */


//
//  Resources: FilesForm
//
#ifndef __WIN32__
// Common
#define FilesFormFilesList                        1000
#define FilesFormPathField                        1002
#define FilesFormSearchButton                     1003
#define FilesFormSearchField                      1004
#define FilesFormCancelButton                     1005
#define FilesFormDeleteButton                     1007
// Drive Form & Load Form
#define FilesFormLoadButton                       1006
// Custom Data
#define FilesFormCustomData                       1008
// Drive Form
#define FilesFormDriveTrigger                     1100
#define FilesFormDrivesList                       1101
#define FilesFormEjectButton                      1102
//
#endif /* __WIN32__ */


//
//  Resources: AboutForm
//
#ifndef __WIN32__
#define AboutFormOKButton                         1200
#define AboutFormVersionLabel                     1206
#endif /* __WIN32__ */


//
//  Resources: DeviceSettingsForm
//
#ifndef __WIN32__
#define DeviceSettingsFormCancelButton            1000
#define DeviceSettingsFormApplyButton             1001
#define DeviceSettingsFormDeviceTrigger           1002
#define DeviceSettingsFormDeviceList              1003
#define DeviceSettingsFormScreenTrigger           1004
#define DeviceSettingsFormScreenList              1005
#define DeviceSettingsFormBrandTrigger            1006
#define DeviceSettingsFormBrandList               1007
#define DeviceSettingsFormIntensitySlider         1008
#define DeviceSettingsFormUseParadosCheck         1009
#define DeviceSettingsFormUse64kMemExtCheck       1010
#define DeviceSettingsFormMode2Antialiased        1011
// Custom Data
#define DeviceSettingsFormCustomData              5000
#endif /* __WIN32__ */


//
//  Resources: EmulatorSettingsForm
//
#ifndef __WIN32__
#define EmulatorSettingsFormCancelButton          1000
#define EmulatorSettingsFormApplyButton           1001
#define EmulatorSettingsFormSystemVolumeCheck     1003
#define EmulatorSettingsFormVolumeSlider          1004
#define EmulatorSettingsFormAutoSaveCheck         1007
#define EmulatorSettingsFormRenderingSlider       1009
#define EmulatorSettingsFormAutoStartCheck        1010
#define EmulatorSettingsFormOnScreenInputTrigger  1011
#define EmulatorSettingsFormOnScreenInputList     1012
// Custom Data
#define EmulatorSettingsFormCustomData            5000
#endif /* __WIN32__ */


//
//  Resources: SaveForm
//
#ifndef __WIN32__
#define SaveFormCancelButton                      1500
#define SaveFormSaveButton                        1501
#define SaveFormNameField                         1502
#define SaveFormExtensionField                    1504
#endif /* __WIN32__ */


//
//  Resources: ScreenOffsetForm
//
#ifndef __WIN32__
#define ScreenOffsetFormCancelButton              1000
#define ScreenOffsetFormApplyButton               1001
#define ScreenOffsetFormOffsetXSlider             1002
#define ScreenOffsetFormResetXButton              1003
#define ScreenOffsetFormOffsetYSlider             1004
#define ScreenOffsetFormResetYButton              1005
#endif /* __WIN32__ */


//
//  Resources: HardKeysForm
//
#ifndef __WIN32__
#define HardKeysFormCancelButton                  1000
#define HardKeysFormApplyButton                   1001
#define HardKeysRockerCenterTrigger               1004
#define HardKeysRockerCenterList                  1005
#define HardKeysHardKey1Trigger                   1006
#define HardKeysHardKey1List                      1007
#define HardKeysHardKey2Trigger                   1008
#define HardKeysHardKey2List                      1009
#define HardKeysHardKey3Trigger                   1010
#define HardKeysHardKey3List                      1011
#define HardKeysHardKey4Trigger                   1012
#define HardKeysHardKey4List                      1013
#define HardKeysCPCKeyCodeAField                  1014
#define HardKeysCPCKeyCodeBField                  1015
#define HardKeysCPCKeyCodeCField                  1016
#define HardKeysCPCKeyCodeDField                  1017
// Custom Data
#define HardKeysFormCustomData                    5000
#endif /* __WIN32__ */


//
// Constant Tables
//
#ifndef __WIN32__
#define TABLE_ID_TYPE                             'tbin'
#define TABLE_ID_DISK_FORMAT                      3000
#define TABLE_ID_COLOURS_RGB                      3001
#define TABLE_ID_COLOURS_GREEN                    3002
#define TABLE_ID_FDC_CMD_TABLE                    3003
#define TABLE_ID_DAATABLE                         3004
#define TABLE_ID_CRC32                            3005
#define TABLE_ID_KEYBOARD_464                     3006
#define TABLE_ID_KEYBOARD_6128                    3007
#define TABLE_ID_MINI_KEYBOARD                    3008
#endif /* __WIN32__ */


//
// Fonts
//
#ifndef __WIN32__
#define FONT_ID_TYPE                              'tbin'
#define FONT_ID_CPC                               6000
#endif /* __WIN32__ */


/* Define the minimum OS version we support */
#ifndef __WIN32__
#define ourMinVersion    sysMakeROMVersion(5,2,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)
#endif /* __WIN32__ */
