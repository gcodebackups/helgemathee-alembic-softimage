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

#ifndef _AlembicTakoImport_AlembicTakoCacheNode_h_
#define _AlembicTakoImport_AlembicTakoCacheNode_h_

#include <AlembicTakoSPIImport/Foundation.h>
#include <AlembicTakoSPIImport/NodeIteratorVisitorHelper.h>

namespace Alembic {

class AlembicTakoCacheNode : public MPxNode
{
public:

    AlembicTakoCacheNode() : mFileInitialized(0), mDebugOn(false)
    {
        mCurFrame = FLT_MAX;

        // 0 mOutPropArrayAttr
        // 1 mOutTransOpArrayAttr, mOutTransOpAngleArrayAttr
        // 2 mOutSubDArrayAttr
        // 3 mOutPolyArrayAttr
        // 4 mOutCameraArrayAttr
        // 5 mOutNurbsSurfaceArrayAttr
        // 6 mOutNurbsCurveGrpArrayAttr
        // 7 mOutParticlePosArrayAttr, mOutParticleIdArrayAttr
        mOutRead = std::vector<bool>(8, false);
    }

    virtual ~AlembicTakoCacheNode() {}

    // avoid calling createSceneVisitor twice by getting the
    // list of hdf reader pointers
    void setReaderPtrList(const WriterData & iData)
    {
        mData = iData;
    }

    static const MTypeId mMayaNodeId;

    // input attributes
    static MObject mSequenceAttr;
    static MObject mTimeAttr;
    static MObject mTakoFileNameAttr;

    static MObject mLoadUVsAttr;
    static MObject mLoadNormalsAttr;

    static MObject mConnectAttr;
    static MObject mCreateIfNotFoundAttr;
    static MObject mRemoveIfNoUpdateAttr;
    static MObject mConnectRootNodesAttr;

    // data attributes
    static MObject mSampledNurbsCurveNumCurveAttr;
    static MObject mIsSampledTransOpAngleAttr;

    // output attributes
    static MObject mOutPropArrayAttr;
    static MObject mOutSubDArrayAttr;
    static MObject mOutPolyArrayAttr;
    static MObject mOutCameraArrayAttr;
    static MObject mOutNurbsCurveGrpArrayAttr;
    static MObject mOutNurbsSurfaceArrayAttr;
    static MObject mOutTransOpArrayAttr;
    static MObject mOutTransOpAngleArrayAttr;

    // override virtual methods from MPxNode
    virtual MStatus compute(const MPlug & plug, MDataBlock & dataBlock);

    // return a pointer to a new instance of the class
    // (derived from MPxNode) that implements the new node type
    static void* creator() { return (new AlembicTakoCacheNode()); }

    // override virtual methods from MPxNode
    virtual bool isPassiveOutput(const MPlug & plug) const;

    // initialize all the attributes to default values
    static MStatus initialize();

    void   setDebugMode(bool iDebugOn){ mDebugOn = iDebugOn; }

private:

    // flag indicating if the input file should be opened again
    bool    mFileInitialized;

    // flag indicating either this is the first time a mesh plug is computed or
    // there's a topology change from last frame to this one
    bool    mSubDInitialized;
    bool    mPolyInitialized;

    double  mSequenceStartFrame;
    double  mSequenceEndFrame;
    float     mCurFrame;

    bool    mDebugOn;

    // bool for each output plug, (the 2 transform plugs are lumped together,
    // when updating) this is to prevent rereading the same
    // frame when above or below the frame range
    std::vector<bool> mOutRead;

    bool    mLoadUVs;
    bool    mLoadNormals;
    MString mCompAttrFileName;

    bool    mConnect;
    bool    mCreateIfNotFound;
    bool    mRemoveIfNoUpdate;
    MString mConnectRootNodes;

    std::set<float> mFrameRange;

    WriterData mData;
};

} // End namespace Alembic

#endif  // _AlembicTakoImport_AlembicTakoCacheNode_h_
