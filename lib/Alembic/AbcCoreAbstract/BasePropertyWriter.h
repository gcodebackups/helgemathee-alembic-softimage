//-*****************************************************************************
//
// Copyright (c) 2009-2010,
//  Sony Pictures Imageworks, Inc. and
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
// Industrial Light & Magic nor the names of their contributors may be used
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

#ifndef _Alembic_AbcCoreAbstract_BasePropertyWriter_h_
#define _Alembic_AbcCoreAbstract_BasePropertyWriter_h_

#include <Alembic/AbcCoreAbstract/Foundation.h>
#include <Alembic/AbcCoreAbstract/ForwardDeclarations.h>
#include <Alembic/AbcCoreAbstract/PropertyType.h>
#include <Alembic/AbcCoreAbstract/MetaData.h>

namespace Alembic {
namespace AbcCoreAbstract {
namespace v1 {

//-*****************************************************************************
//! The BasePropertyWriter is the base class for all properties,
//! from Simple (which encompasses Scalar and Array properties) to
//! Compound. The functions here establish the things which all properties
//! have in common - name, metaData, propertyType, as well as
//! upcasting capabilities. We don't want to rely on dynamic_cast - it's
//! slow and potentially has problems across DSO interfaces.
class BasePropertyWriter
    : private boost::noncopyable,
      protected boost::enable_shared_from_this<BasePropertyWriter>
{
public:
    //! Virtual destructor
    //! ...
    virtual ~BasePropertyWriter();
    
    //-*************************************************************************
    // NEW FUNCTIONS
    //-*************************************************************************

    //! All properties have a name, which is unique amongst its siblings
    //! in the compund property they all live in. This is the name that
    //! was given when the property was created.
    virtual const std::string &getName() const = 0;

    //! There are three types of abstract properties.
    //! They are Scalar, Array, and Compound properties. This function
    //! returns an enum \ref PropertyType which indicates which property
    //! type is returned.
    virtual PropertyType getPropertyType() const = 0;

    //! Convenience to return whether the property is scalar.
    //! Same as getPropertyType() == kScalarProperty
    bool isScalar() const { return getPropertyType() == kScalarProperty; }

    //! Convenience to return whether the property is array.
    //! Same as getPropertyType() == kArrayProperty
    bool isArray() const { return getPropertyType() == kArrayProperty; }

    //! Convenience to return whether the property is compound.
    //! Same as getPropertyType() == kCompoundProperty
    bool isCompound() const { return getPropertyType() == kCompoundProperty; }

    //! Convenience to return whether the property is simple (non-compound)
    //! Same as getPropertyType() != kCompoundProperty
    bool isSimple() const { return !isCompound(); }

    //! All properties have metadata. This metadata is identical to the
    //! Because the metadata must exist and be initialized in order to
    //! bootstrap the property, it is guaranteed to exist and is returned
    //! by reference. While MetaData was required to create the property,
    //! additional MetaData may be appended during the writing of the
    //! property. This returns the accumulated MetaData, which may change
    //! as writing occurs. The reference will remain valid, but the
    //! MetaData it points to may change over time.
    virtual const MetaData &getMetaData() const = 0;
    
    //! All properties have an object that owns them, and in order to
    //! ensure the object stays alive as long as the properties do, they
    //! retain a shared pointer to their object.
    virtual ObjectWriterPtr getObject() = 0;

    //! Most properties live in a compound property. (Except for
    //! the top-compound property in any object)
    //! This returns a pointer to the parent compound property.
    virtual CompoundPropertyWriterPtr getParent() = 0;
    
    //! Append metadata to what already exists. Any new metadata must
    //! not overlap with existing metadata. An exception will be thrown
    //! if any MetaData overlaps.
    virtual void appendMetaData( const MetaData &iAppend ) = 0;
    
    //! Return shared pointer to self, assuming we are always
    //! created with one. This is non-virtual.
    BasePropertyWriterPtr asBase();

    //! Up-cast this base property to a SimpleProperty, if such an
    //! upcast is valid. This can be checked with the \ref isSimple()
    //! function. If the upcast is not valid, an empty pointer will
    //! be returned. This default implementation returns an empty
    //! pointer.
    virtual SimplePropertyWriterPtr asSimple();

    //! Up-cast this base property to a ScalarProperty, if such an
    //! upcast is valid. This can be checked with the \ref isScalar()
    //! function. If the upcast is not valid, an empty pointer will
    //! be returned. This default implementation returns an empty
    //! pointer.
    virtual ScalarPropertyWriterPtr asScalar();
    
    //! Up-cast this base property to an ArrayProperty, if such an
    //! upcast is valid. This can be checked with the \ref isArray()
    //! function. If the upcast is not valid, an empty pointer will
    //! be returned. This default implementation returns an empty
    //! pointer.
    virtual ArrayPropertyWriterPtr asArray();
    
    //! Up-cast this base property to a CompoundProperty, if such an
    //! upcast is valid. This can be checked with the \ref isCompund()
    //! function. If the upcast is not valid, an empty pointer will
    //! be returned. This default implementation returns an empty
    //! pointer.
    virtual CompoundPropertyWriterPtr asCompound();
};

} // End namespace v1
} // End namespace AbcCoreAbstract
} // End namespace Alembic

#endif
