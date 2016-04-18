#include "masterarmconfig.h"

const char *_tag_YawMin = "YawMin";
const char *_tag_YawMax = "YawMax";
const char *_tag_YawAdd = "YawAdd";
const char *_tag_YawReverse = "YawReverse";
const char *_tag_ShoulderMin = "ShoulderMin";
const char *_tag_ShoulderMax = "ShoulderMax";
const char *_tag_ShoulderAdd = "ShoulderAdd";
const char *_tag_ShoulderReverse = "ShoulderReverse";
const char *_tag_ElbowMin = "ElbowMin";
const char *_tag_ElbowMax = "ElbowMax";
const char *_tag_ElbowAdd = "ElbowAdd";
const char *_tag_ElbowReverse = "ElbowReverse";
const char *_tag_WristMin = "WristMin";
const char *_tag_WristMax = "WristMax";
const char *_tag_WristAdd = "WristAdd";
const char *_tag_WristReverse = "WristReverse";

namespace Soro {

bool MasterArmConfig::load(QFile& file) {
    IniParser parser;
    if (!parser.load(file)) return false;
    //parse min/max values
    bool success = parser.valueAsInt(_tag_YawMin, &yawMin);
    success &= parser.valueAsInt(_tag_YawMax, &yawMax);
    success &= parser.valueAsInt(_tag_ShoulderMin, &shoulderMin);
    success &= parser.valueAsInt(_tag_ShoulderMin, &shoulderMax);
    success &= parser.valueAsInt(_tag_ElbowMin, &elbowMin);
    success &= parser.valueAsInt(_tag_ElbowMax, &elbowMax);
    success &= parser.valueAsInt(_tag_WristMin, &wristMin);
    success &= parser.valueAsInt(_tag_WristMax, &wristMax);
    //parse add values (defaults to 0 if not found)
    if (!parser.valueAsInt(_tag_YawAdd, &yawAdd)) yawAdd = 0;
    if (!parser.valueAsInt(_tag_ShoulderAdd, &shoulderAdd)) shoulderAdd = 0;
    if (!parser.valueAsInt(_tag_ElbowAdd, &elbowAdd)) elbowAdd = 0;
    if (!parser.valueAsInt(_tag_WristAdd, &wristAdd)) wristAdd = 0;
    //parse reverse values (defaults to false if not found)
    if (!parser.valueAsBool(_tag_YawReverse, &yawReverse)) yawReverse = false;
    if (!parser.valueAsBool(_tag_ShoulderReverse, &shoulderReverse)) shoulderReverse = false;
    if (!parser.valueAsBool(_tag_ElbowReverse, &elbowReverse)) elbowReverse = false;
    if (!parser.valueAsBool(_tag_WristReverse, &wristReverse)) wristReverse = false;
    return success;
}

}
