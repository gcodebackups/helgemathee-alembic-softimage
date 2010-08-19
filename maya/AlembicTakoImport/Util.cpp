//-*****************************************************************************
//
// Copyright (c) 2009-2010,
//  Sony Pictures Imageworks Inc. and
//  Industrial Light & Magic, a division of Lucasfilm Entertainment Company Ltd.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Sony Pictures Imageworks, nor
// Industrial Light & Magic, nor the names of their contributors may be used
// to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//-*****************************************************************************


#include <AlembicTakoImport/Util.h>

namespace Alembic {

float getValidCurrentFrame(float inputTime,
  const std::set<float> & iFrameRange)
{
    // safety check
    // ideally this function shouldn't be called if the scene is static
    if (iFrameRange.empty())
        return inputTime;

    float start = *(iFrameRange.begin());
    if (inputTime <= start)
        return start;

    std::set<float>::iterator endIt = iFrameRange.end();
    std::set<float>::iterator it = iFrameRange.lower_bound(inputTime);

    if (it == endIt)
        return *(--it);

    float hi = *it;
    float lo = *(--it);

    if (hi-inputTime < 1e-2)
        return hi;

    return lo;
}

void getTakoFrameRange(const Tako::TransformReaderPtr & iRootNode,
    double & oSeqStartFrame, double & oSeqEndFrame,
    std::set<float> & oFrameRange)
{
    oSeqStartFrame = 0.0;
    oSeqEndFrame = 0.0;
    oFrameRange = std::set<float>();
    std::vector<float> frames;

    Tako::PropertyPair p;
    if (iRootNode->readProperty("frameRange", 0.0, p) != Tako::PROPERTY_NONE)
    {
        frames = boost::get< std::vector<float> >(p.first);
    }
    else if (iRootNode->readProperty("frameRangeShape", 0.0, p)
        != Tako::PROPERTY_NONE)
    {
        frames = boost::get< std::vector<float> >(p.first);
    }
    else if (iRootNode->readProperty("frameRangeTrans", 0.0, p)
        != Tako::PROPERTY_NONE)
    {
        frames = boost::get< std::vector<float> >(p.first);
    }

    // the frames are written out in ascending order
    if (!frames.empty())
    {
        oSeqStartFrame = frames[0];
        oSeqEndFrame = frames[frames.size()-1];
        oFrameRange.insert(frames.begin(), frames.end());
    }
}

MStatus replaceDagObject(MObject & oldObject, MObject & newObject,
            const MString & name)
{
    MStatus status;
    MFnDagNode mFnOld(oldObject, &status);
    if (status == MS::kSuccess)
    {
        unsigned int numChild = mFnOld.childCount();

        std::vector<MObject> children;
        children.reserve(numChild);
        for (unsigned int i = 0; i < numChild; i++)
        {
            MObject child = mFnOld.child(i, &status);
            if (status == MS::kSuccess)
            {
                children.push_back(child);
            }
            else
            {
                MString theError("Failed to get child ");
                theError += i;
                theError += " of ";
                theError += mFnOld.name();
                theError += ", status = ";
                theError += status.errorString();
                MGlobal::displayError(theError);
            }
        }

        MFnDagNode mFnNew(newObject, &status);
        if (status == MS::kSuccess)
        {
            for (unsigned int i = 0; i < numChild; i++)
            {
                status = mFnNew.addChild(children[i]);
                if (status != MS::kSuccess)
                {
                    MString theError("Failed to add child ");
                    theError += i;
                    theError += " of ";
                    theError += mFnOld.name();
                    theError += " to ";
                    theError += name;
                    theError += ", status = ";
                    theError += status.errorString();
                    MGlobal::displayError(theError);
                }
            }
        }
    }
    return status;
}

MStatus disconnectAllPlugsTo(MPlug & dstPlug)
{
    MStatus status = MS::kSuccess;
    MPlugArray array;
    dstPlug.connectedTo(array, true, false, &status);
    unsigned int arrayLength = array.length();

    for (unsigned int i = 0; i < arrayLength; i++)
    {
        MPlug srcPlug = array[i];
        if (status == MS::kSuccess)
        {
            MDGModifier modifier;
            status = modifier.disconnect(srcPlug, dstPlug);
            status = modifier.doIt();
            if (status != MS::kSuccess)
            {
                MString theError("Disconnect ");
                theError += srcPlug.name();
                theError += MString(" -> ");
                theError += dstPlug.name();
                theError += MString(" failed, status = ");
                theError += status.errorString();
                MGlobal::displayError(theError);
                return status;
            }
        }
    }
    return MS::kSuccess;
}

MStatus getDagPathListByName(const MString & objectNames,
    std::vector<MDagPath> & dagPathList)
{
    MStatus status;
    MStringArray theArray;
    status = objectNames.split(' ', theArray);
    if (status == MS::kSuccess)
    {
        unsigned int len = theArray.length();
        for (unsigned int i = 0; i < len; i++)
        {
            MDagPath dagPath;
            status = getDagPathByName(theArray[i], dagPath);
            if (status == MS::kSuccess)
                dagPathList.push_back(dagPath);
            else
            {
                MString theError(theArray[i]);
                theError += MString(" doesn't exist");
                MGlobal::displayError(theError);
            }
        }
    }

    return status;
}

MStatus getObjectByName(const MString & name, MObject & object)
{
    object = MObject::kNullObj;

    MSelectionList sList;
    MStatus status = sList.add(name);
    if (status == MS::kSuccess)
    {
        status = sList.getDependNode(0, object);
    }

    return status;
}

MStatus getDagPathByName(const MString & name, MDagPath & dagPath)
{
    MSelectionList sList;
    MStatus status = sList.add(name);
    if (status == MS::kSuccess)
    {
        status = sList.getDagPath(0, dagPath);
    }

    return status;
}

MStatus getPlugByName(const MString & objName, const MString & attrName,
            MPlug & plug)
{
    MObject object = MObject::kNullObj;
    MStatus status = getObjectByName(objName, object);
    if (status == MS::kSuccess)
    {
        MFnDependencyNode mFn(object, &status);
        if (status == MS::kSuccess)
            plug = mFn.findPlug(attrName, &status);
    }

    return status;
}

MStatus setPlayback(double min, double max, double cur)
{
    validCur(min, max, cur);

    MStatus status = MS::kSuccess;
    MAnimControl anim;
    MTime minTime, maxTime, curTime;
    minTime.setValue(min);
    maxTime.setValue(max);

    status = anim.setMinTime(minTime);
    status = anim.setAnimationStartTime(minTime);

    if (max > 0)
    {
        status = anim.setMaxTime(maxTime);
        status = anim.setAnimationEndTime(maxTime);
    }

    // status = MGlobal::viewFrame(cur);

    return status;
}

MStatus setInitialShadingGroup(const MString & dagNodeName)
{
    MObject initShader;
    MDagPath dagPath;

    if (getObjectByName("initialShadingGroup", initShader) == MS::kSuccess &&
        getDagPathByName(dagNodeName, dagPath) == MS::kSuccess)
    {
        MFnSet set(initShader);
        set.addMember(dagPath);
    }
    else
    {
        MString theError("Error getting adding ");
        theError += dagNodeName;
        theError += MString(" to initalShadingGroup.");
        MGlobal::displayError(theError);
        return MS::kFailure;
    }
    return MS::kSuccess;
}

MStatus deleteDagNode(MDagPath & dagPath)
{
    MObject obj = dagPath.node();
    return MGlobal::deleteNode(obj);
}

MStatus deleteCurrentSelection()
{
    MStatus status;

    MDGModifier modifier;
    status = modifier.commandToExecute("\ndelete;\n");
    status = modifier.doIt();

    return status;
}

bool stripTakoFileName(const MString & FilePath, MString & FileName)
{
    std::string str(FilePath.asChar());
    size_t found;
    found = str.find_last_of("/\\");
    str = str.substr(found+1);
    // str is now in the form of xxx.hdf
    found = str.find_first_of('.');
    str = str.substr(0, found);
    FileName = MString(str.c_str());

    return true;
}

bool removeDangleAlembicTakoCacheNodes()
{
    MStringArray result;
    MStatus status =
        MGlobal::executeCommand("ls -type AlembicTakoCacheNode", result, true);

    if (status != MS::kSuccess ) return false;

    unsigned int numOfNodes = result.length();
    for ( unsigned int n = 0; n < numOfNodes; n++ )
    {
        MObject obj;
        getObjectByName(result[n], obj);
        MFnDependencyNode mFn(obj);
        MPlugArray array;
        status = mFn.getConnections(array);
        if (status == MS::kSuccess && array.length() == 1)
        {
            MGlobal::deleteNode(obj);
        }
    }

    return true;
}

void setDebug(bool debug)
{
    // do nothing
}

} // End namespace Alembic


