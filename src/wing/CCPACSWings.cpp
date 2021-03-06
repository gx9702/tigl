/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file 
* @brief  Implementation of CPACS wings handling routines.
*/

#include "CCPACSWings.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSWings::CCPACSWings(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSWings::~CCPACSWings(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWings::Invalidate(void)
{
    profiles.Invalidate();
    for (int i = 1; i <= GetWingCount(); i++) {
        GetWing(i).Invalidate();
    }
}

// Cleanup routine
void CCPACSWings::Cleanup(void)
{
    for (CCPACSWingContainer::size_type i = 0; i < wings.size(); i++) {
        delete wings[i];
    }
    wings.clear();
}

void CCPACSWings::ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID)
{
    Cleanup();

    profiles.ReadCPACS(tixiHandle);

    // read normal wings
    ReadCPACSWing(tixiHandle, configurationUID, "wings", "wing");

    if (configuration->IsRotorcraft()) {
         // read rotor blades
         ReadCPACSWing(tixiHandle, configurationUID, "rotorBlades", "rotorBlade");
    }
}

// Read CPACS wings element
void CCPACSWings::ReadCPACSWing(TixiDocumentHandle tixiHandle, const char* configurationUID, const char* wingsLibraryName, const char* wingElementName)
{
    char *tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSWings::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string wingXPath = tmpString;
    wingXPath += "[@uID=\"";
    wingXPath += configurationUID;
    wingXPath += "\"]/";
    wingXPath += wingsLibraryName;

    if (tixiCheckElement(tixiHandle, wingXPath.c_str()) != SUCCESS) {
        return;
    }

    /* Get wing element count */
    int wingCount;
    if (tixiGetNamedChildrenCount(tixiHandle, wingXPath.c_str(), wingElementName, &wingCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWings::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all wings
    for (int i = 1; i <= wingCount; i++) {
        CCPACSWing* wing = new CCPACSWing(configuration);
        wings.push_back(wing);

        std::ostringstream xpath;
        xpath << wingXPath << "/" << wingElementName << "[" << i << "]";
        wing->ReadCPACS(tixiHandle, xpath.str());
    }
}

bool CCPACSWings::HasProfile(std::string uid) const
{
    return profiles.HasProfile(uid);
}

// Returns the total count of wing profiles in this configuration
int CCPACSWings::GetProfileCount(void) const
{
    return profiles.GetProfileCount();
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWings::GetProfile(std::string uid) const
{
    return profiles.GetProfile(uid);
}

// Returns the wing profile for a given index.
CCPACSWingProfile& CCPACSWings::GetProfile(int index) const
{
    return profiles.GetProfile(index);
}

// Returns the total count of wings in a configuration
int CCPACSWings::GetWingCount(void) const
{
    return (static_cast<int>(wings.size()));
}

// Returns the count of wings in a configuration with the property isRotorBlade set to true
int CCPACSWings::GetRotorBladeCount(void) const
{
    int nRotorBlades = 0;
    for (int i = 1; i <= GetWingCount(); i++) {
        if (GetWing(i).IsRotorBlade()) {
            nRotorBlades++;
        }
    }
    return nRotorBlades;
}

// Returns the wing for a given index.
CCPACSWing& CCPACSWings::GetWing(int index) const
{
    index --;
    if (index < 0 || index >= GetWingCount()) {
        throw CTiglError("Error: Invalid index in CCPACSWings::GetWing", TIGL_INDEX_ERROR);
    }
    return (*wings[index]);
}

// Returns the wing for a given UID.
CCPACSWing& CCPACSWings::GetWing(const std::string& UID) const
{
    return (*wings[GetWingIndex(UID)-1]);
}

// Returns the wing index for a given UID.
int CCPACSWings::GetWingIndex(const std::string& UID) const
{
    for (int i=0; i < GetWingCount(); i++) {
        const std::string tmpUID(wings[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSWings::GetWingIndex", TIGL_UID_ERROR);
}

} // end namespace tigl
