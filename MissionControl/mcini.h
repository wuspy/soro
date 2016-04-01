#ifndef MCINI_H
#define MCINI_H

#include <QtCore>

#include "iniparser.h"
#include "logger.h"
#include "socketaddress.h"

#define MCINI_PATH "config/mission_control.ini"
#define GLFWINI_PATH "config/last_glfw_config.ini"
#define MCINI_TAG_LAYOUT "Layout"
#define MCINI_VALUE_LAYOUT_ARM "arm"
#define MCINI_VALUE_LAYOUT_DRIVE "drive"
#define MCINI_VALUE_LAYOUT_GIMBAL "gimbal"
#define MCINI_VALUE_LAYOUT_SPECTATOR "spectator"
#define MCINI_TAG_COMM_HOST_ADDRESS "MainHostAddress"
#define MCINI_TAG_VIDEO_HOST_ADDRESS "VideoHostAddress"
#define MCINI_TAG_INPUT_MODE "InputMode"
#define MCINI_VALUE_USE_GLFW "glfw"
#define MCINI_VALUE_USE_MASTER "masterarm"
#define MCINI_TAG_MASTER_NODE "MasterNode"
#define MCINI_TAG_MASTER_NODE_ADDRESS "MasterNodeAddress"
#define MCINI_TAG_MASTER_NODE_PORT "MasterNodePort"
#define MCINI_TAG_MASTER_NODE_PORT_RANGE "NodePorts"
#define MCINI_TAG_NODE_HOST_ADDRESS "NodeHostAddress"

namespace Soro {

/* Struct to contain and parse the configuration file that is common
 * to mission control and the rover (stuff like the server IP, which side should
 * act as server, ports, etc)
 */
struct MissionControlIniConfig {
    enum InputMode {
        GLFW, MasterArm
    };

    enum LayoutMode {
        ArmLayoutMode, DriveLayoutMode, GimbalLayoutMode, SpectatorLayoutMode
    };

    ~MissionControlIniConfig() {
        if (NodePorts != NULL) delete[] NodePorts;
    }

    QHostAddress CommHostAddress;
    QHostAddress VideoHostAddress;
    QHostAddress NodeHostAddress;
    InputMode ControlInputMode;
    LayoutMode Layout;
    bool MasterNode;
    quint16 *NodePorts = NULL;
    SocketAddress MasterNodeAddress;

    /* Loads the configuration from the default path, and returns true if successful.
     * If there is an error, err will contain a summary of what went wrong.
     */
    bool load(QString *err) {
        QString appPath = QCoreApplication::applicationDirPath();
        IniParser configParser;
        QFile configFile(appPath + "/" + MCINI_PATH);
        if (!configParser.load(configFile)) {
            *err = "The configuration file " + appPath + "/" + MCINI_PATH + " missing or invalid";
            return false;
        }
        if (!configParser.valueAsIP(MCINI_TAG_COMM_HOST_ADDRESS, &CommHostAddress, true)) {
            //use default host address
            CommHostAddress = QHostAddress::Any;
        }
        if (!configParser.valueAsIP(MCINI_TAG_VIDEO_HOST_ADDRESS, &VideoHostAddress, true)) {
            //use communication host as video host
            VideoHostAddress = QHostAddress::Any;
        }
        if (!configParser.valueAsBool(MCINI_TAG_MASTER_NODE, &MasterNode)) {
            *err = "The configuration file " + appPath + "/" + MCINI_PATH + " is invalid (can't determine master node value)";
            return false;
        }
        if (!configParser.valueAsIP(MCINI_TAG_NODE_HOST_ADDRESS, &NodeHostAddress, true)) {
            //use default address for mission control network
            NodeHostAddress = QHostAddress::Any;
        }
        if (MasterNode) {
            int *nodePorts = new int[2];
            if (!configParser.valueAsIntRange(MCINI_TAG_MASTER_NODE_PORT_RANGE, nodePorts)) {
                *err = "The configuration file " + appPath + "/" + MCINI_PATH + " is invalid (can't determine master node's open ports')";
                return false;
            }
            NodePorts = new quint16[2];
            NodePorts[0] = nodePorts[0];
            NodePorts[1] = nodePorts[1];
            delete[] nodePorts;
        }
        else {
            if (!configParser.valueAsIP(MCINI_TAG_MASTER_NODE_ADDRESS, &MasterNodeAddress.address, true)) {
                *err = "The configuration file " + appPath + "/" + MCINI_PATH + " is invalid (can't determine master node address)";
                return false;
            }
            int port;
            if (!configParser.valueAsInt(MCINI_TAG_MASTER_NODE_PORT, &port)) {
                *err = "The configuration file " + appPath + "/" + MCINI_PATH + " is invalid (can't determine master node port)";
                return false;
            }
            MasterNodeAddress.port = (quint16)port;
        }
        QString modeStr = configParser.value(MCINI_TAG_LAYOUT);
        if (QString::compare(modeStr, MCINI_VALUE_LAYOUT_ARM, Qt::CaseInsensitive) == 0) {
            Layout = ArmLayoutMode;
            QString inputMode = configParser.value(MCINI_TAG_INPUT_MODE);
            if (QString::compare(inputMode, MCINI_VALUE_USE_GLFW, Qt::CaseInsensitive) == 0) {
                //use gamepad to control the arm
                ControlInputMode = GLFW;
            }
            else if (QString::compare(inputMode, MCINI_VALUE_USE_MASTER, Qt::CaseInsensitive) == 0) {
                //Use the master/slave arm input method
                ControlInputMode = MasterArm;
            }
            else {
                *err = "The configuration file " + appPath + "/" + MCINI_PATH + " is invalid (can't determine input mode)";
                return false;
            }
        }
        else if ((QString::compare(modeStr, MCINI_VALUE_LAYOUT_DRIVE, Qt::CaseInsensitive) == 0)) {
            Layout = DriveLayoutMode;
        }
        else if (QString::compare(modeStr, MCINI_VALUE_LAYOUT_GIMBAL, Qt::CaseInsensitive) == 0) {
            Layout = GimbalLayoutMode;
        }
        else if (QString::compare(modeStr, MCINI_VALUE_LAYOUT_SPECTATOR, Qt::CaseInsensitive) == 0) {
            Layout = SpectatorLayoutMode;
        }
        else {
            *err = "The configuration file " + appPath + "/" + MCINI_PATH + " is invalid (can't determine layout)";
            return false;
        }
        return true;
    }
};

}

#endif // MCINI_H
