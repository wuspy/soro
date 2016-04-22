#include "mcini.h"

const char *_tag_Layout = "Layout";
const char *_value_UseArmLayout = "arm";
const char *_value_UseDriveLayout = "drive";
const char *_value_UseGimbalLayout = "gimbal";
const char *_value_UseSpectatorLayout = "spectator";
const char *_tag_InputMode = "InputMode";
const char *_value_UseGamepadInput = "Gamepad";
const char *_value_UseMasterArmInput = "MasterArm";
const char *_tag_MasterArmPort = "MasterArmPort";
const char *_tag_IsMasterNode = "MasterSubnetNode";

namespace Soro {
namespace MissionControl {

/* Loads the configuration from the default path, and returns true if successful.
 * If there is an error, err will contain a summary of what went wrong.
 */
bool MissionControlIniLoader::load(QString *err) {
    IniParser configParser;
    QFile configFile(MISSION_CONTROL_INI_PATH);
    if (!configParser.load(configFile)) {
        *err = "The configuration file " +  MISSION_CONTROL_INI_PATH + " missing or invalid";
        return false;
    }
    if (!configParser.valueAsBool(_tag_IsMasterNode, &MasterNode)) {
        *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine master node value)";
        return false;
    }
    QString modeStr = configParser.value(_tag_Layout);
    ControlInputMode = Gamepad; //default
    if (QString::compare(modeStr, _value_UseArmLayout, Qt::CaseInsensitive) == 0) {
        Layout = ArmLayoutMode;
        QString inputMode = configParser.value(_tag_InputMode);
        if (QString::compare(inputMode, _value_UseGamepadInput, Qt::CaseInsensitive) == 0) {
            //use gamepad to control the arm
            ControlInputMode = Gamepad;
        }
        else if (QString::compare(inputMode, _value_UseMasterArmInput, Qt::CaseInsensitive) == 0) {
            //Use the master/slave arm input method
            ControlInputMode = MasterArm;
            int tmp;
            if (!configParser.valueAsInt(_tag_MasterArmPort, &tmp)) {
                *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine master arm port)";
                return false;
            }
            MasterArmPort = (quint16)tmp;
        }
        else {
            *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine input mode)";
            return false;
        }
    }
    else if ((QString::compare(modeStr, _value_UseDriveLayout, Qt::CaseInsensitive) == 0)) {
        Layout = DriveLayoutMode;
    }
    else if (QString::compare(modeStr, _value_UseGimbalLayout, Qt::CaseInsensitive) == 0) {
        Layout = GimbalLayoutMode;
    }
    else if (QString::compare(modeStr, _value_UseSpectatorLayout, Qt::CaseInsensitive) == 0) {
        Layout = SpectatorLayoutMode;
    }
    else {
        *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine layout)";
        return false;
    }
    return true;
}

}
}
