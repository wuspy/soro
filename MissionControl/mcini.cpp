#include "mcini.h"

const char *_tag_Layout = "Layout";
const char *_value_UseArmLayout = "arm";
const char *_value_UseDriveLayout = "drive";
const char *_value_UseGimbalLayout = "gimbal";
const char *_value_UseSpectatorLayout = "spectator";
const char *_tag_CommunicationHostAddress = "MainHostAddress";
const char *_tag_VideoHostAddress = "VideoHostAddress";
const char *_tag_InputMode = "InputMode";
const char *_value_UseGamepadInput = "Gamepad";
const char *_value_UseMasterArmInput = "MasterArm";
const char *_tag_MasterArmPort = "MasterArmPort";
const char *_tag_IsMasterNode = "MasterNode";
const char *_tag_MasterNodeAddress = "MasterNodeAddress";
const char *_tag_MasterNodePort = "MasterNodePort";
const char *_tag_MasterNodePortRange = "NodePorts";
const char *_tag_NodeHostAddress = "NodeHostAddress";

namespace Soro {
namespace MissionControl {

MissionControlIniLoader::~MissionControlIniLoader() {
    if (NodePorts != NULL) delete[] NodePorts;
}

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
    if (!configParser.valueAsIP(_tag_CommunicationHostAddress, &CommHostAddress, true)) {
        //use default host address
        CommHostAddress = QHostAddress::Any;
    }
    if (!configParser.valueAsIP(_tag_VideoHostAddress, &VideoHostAddress, true)) {
        //use communication host as video host
        VideoHostAddress = QHostAddress::Any;
    }
    if (!configParser.valueAsBool(_tag_IsMasterNode, &MasterNode)) {
        *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine master node value)";
        return false;
    }
    if (!configParser.valueAsIP(_tag_NodeHostAddress, &NodeHostAddress, true)) {
        //use default address for mission control network
        NodeHostAddress = QHostAddress::Any;
    }
    if (MasterNode) {
        int *nodePorts = new int[2];
        if (!configParser.valueAsIntRange(_tag_MasterNodePortRange, nodePorts)) {
            *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine master node's open ports')";
            return false;
        }
        NodePorts = new quint16[2];
        NodePorts[0] = nodePorts[0];
        NodePorts[1] = nodePorts[1];
        delete[] nodePorts;
    }
    else {
        if (!configParser.valueAsIP(_tag_MasterNodeAddress, &MasterNodeAddress.host, true)) {
            *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine master node address)";
            return false;
        }
        int port;
        if (!configParser.valueAsInt(_tag_MasterNodePort, &port)) {
            *err = "The configuration file " + MISSION_CONTROL_INI_PATH + " is invalid (can't determine master node port)";
            return false;
        }
        MasterNodeAddress.port = (quint16)port;
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
