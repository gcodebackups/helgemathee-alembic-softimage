// alembicPlugin
// Initial code generated by Softimage SDK Wizard
// Executed Fri Aug 19 09:14:49 UTC+0200 2011 by helge
// 
// Tip: You need to compile the generated code before you can load the plug-in.
// After you compile the plug-in, you can load it by clicking Update All in the Plugin Manager.
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_menu.h>
#include <xsi_uitoolkit.h>
#include <xsi_comapihandler.h>
#include <xsi_project.h>
#include <xsi_selection.h>
#include <xsi_model.h>
#include <xsi_camera.h>
#include <xsi_customoperator.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_factory.h>
#include <xsi_primitive.h>
#include <xsi_math.h>

using namespace XSI; 
using namespace MATH; 

#include "AlembicWriteJob.h"

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"helge");
	in_reg.PutName(L"alembicPlugin");
	in_reg.PutVersion(1,0);
	in_reg.RegisterCommand(L"alembic_export",L"alembic_export");
	in_reg.RegisterCommand(L"alembic_import",L"alembic_import");
	in_reg.RegisterOperator(L"alembic_xform");
	in_reg.RegisterOperator(L"alembic_camera");
	in_reg.RegisterOperator(L"alembic_polymesh");
	in_reg.RegisterMenu(siMenuMainFileExportID,L"alembic_MenuExport",false,false);
	in_reg.RegisterMenu(siMenuMainFileImportID,L"alembic_MenuImport",false,false);
	//RegistrationInsertionPoint - do not remove this line

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
   deleteAllArchives();

	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}

SICALLBACK alembic_export_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);

	ArgumentArray oArgs;
	oArgs = oCmd.GetArguments();
	oArgs.Add(L"filename");
	oArgs.AddWithHandler(L"objects",L"Collection");
	oArgs.Add(L"frameIn",1.0);
	oArgs.Add(L"frameOut",100.0);
	oArgs.Add(L"frameStep",1.0);
	return CStatus::OK;
}

SICALLBACK alembic_export_Execute( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	CValueArray args = ctxt.GetAttribute(L"Arguments");
	
   // take care of the filename
   CString filename = (CString)args[0].GetAsText();
   if(filename.IsEmpty())
   {
      // let's see if we are in interactive mode
      if(Application().IsInteractive())
      {
         CComAPIHandler toolkit;
         toolkit.CreateInstance(L"XSI.UIToolkit");
         CComAPIHandler filebrowser(toolkit.GetProperty(L"FileBrowser"));
         filebrowser.PutProperty(L"InitialDirectory",Application().GetActiveProject().GetPath());
         filebrowser.PutProperty(L"Filter",L"Alembic Files(*.abc)|*.abc||");
         CValue returnVal;
         filebrowser.Call(L"ShowSave",returnVal);
         filename = filebrowser.GetProperty(L"FilePathName").GetAsText();
         if(filename.IsEmpty())
            return CStatus::Abort;
      }
      else
      {
         Application().LogMessage(L"[alembic] No filename specified.",siErrorMsg);
         return CStatus::InvalidArgument;
      }
   }
   Application().LogMessage(L"[alembic] filename used: "+filename);

   // check the objects
	CRefArray objects = (CRefArray)args[1];
   if(objects.GetCount() == 0)
   {
      // use the selection
      objects = Application().GetSelection().GetArray();
      if(objects.GetCount() == 0)
      {
         Application().LogMessage(L"[alembic] No objects specified.",siErrorMsg);
         return CStatus::InvalidArgument;
      }
   }
   Application().LogMessage(L"[alembic] objects used: "+CValue(objects.GetCount()).GetAsText());

   // check the frames
   double frameIn = (double)args[2];
   double frameOut = (double)args[3];
   double frameStep = (double)args[4];
   CDoubleArray frames;
   for(double frame=frameIn; frame<=frameOut; frame+=frameStep)
      frames.Add(frame);
   Application().LogMessage(L"[alembic] frames used: "+CValue(frameIn).GetAsText()+L" to "+CValue(frameOut).GetAsText()+L", step "+CValue(frameStep).GetAsText());

   // run the job
   AlembicWriteJob job(filename,objects,frames);
	return job.Process();
}

SICALLBACK alembic_import_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);

	ArgumentArray oArgs;
	oArgs = oCmd.GetArguments();
	oArgs.Add(L"filename");
	return CStatus::OK;
}

SICALLBACK alembic_import_Execute( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	CValueArray args = ctxt.GetAttribute(L"Arguments");
	
   // take care of the filename
   CString filename = (CString)args[0].GetAsText();
   if(filename.IsEmpty())
   {
      // let's see if we are in interactive mode
      if(Application().IsInteractive())
      {
         CComAPIHandler toolkit;
         toolkit.CreateInstance(L"XSI.UIToolkit");
         CComAPIHandler filebrowser(toolkit.GetProperty(L"FileBrowser"));
         filebrowser.PutProperty(L"InitialDirectory",Application().GetActiveProject().GetPath());
         filebrowser.PutProperty(L"Filter",L"Alembic Files(*.abc)|*.abc||");
         CValue returnVal;
         filebrowser.Call(L"ShowOpen",returnVal);
         filename = filebrowser.GetProperty(L"FilePathName").GetAsText();
         if(filename.IsEmpty())
            return CStatus::Abort;
      }
      else
      {
         Application().LogMessage(L"[alembic] No filename specified.",siErrorMsg);
         return CStatus::InvalidArgument;
      }
   }
   Application().LogMessage(L"[alembic] filename used: "+filename);

   // let's try to read this
   Alembic::Abc::IArchive * archive = new Alembic::Abc::IArchive( Alembic::AbcCoreHDF5::ReadArchive(), filename.GetAsciiString() );

   // let's figure out which objects we have
   CRefArray ops;
   std::vector<Alembic::Abc::IObject> objects;
   objects.push_back(archive->getTop());
   for(size_t i=0;i<objects.size();i++)
   {
      // first, let's recurse
      for(size_t j=0;j<objects[i].getNumChildren();j++)
         objects.push_back(objects[i].getChild(j));

      // now let's see what we have here
      if(Alembic::AbcGeom::ICamera::matches(objects[i].getMetaData()))
      {
         Alembic::Abc::IObject parent = objects[i].getParent();
         std::string name = objects[i].getName();
         if(Alembic::AbcGeom::IXform::matches(parent.getMetaData()))
            name = parent.getName();

         Alembic::AbcGeom::ICamera meshIObject(objects[i],Alembic::Abc::kWrapExisting);

         // let's create a camera
         Camera camera;
         Application().GetActiveSceneRoot().AddCamera(L"Camera",name.c_str(),camera);

         // let's setup the xform op
         if(Alembic::AbcGeom::IXform::matches(parent.getMetaData()))
         {
            CustomOperator op = Application().GetFactory().CreateObject(L"alembic_xform");
            op.AddOutputPort(camera.GetKinematics().GetGlobal().GetRef());
            op.AddInputPort(camera.GetKinematics().GetGlobal().GetRef());
            op.Connect();
            ops.Add(op.GetRef());
            op.PutParameterValue(L"path",filename);
            op.PutParameterValue(L"identifier",CString(parent.getFullName().c_str()));
         }
         
         // let's setup the camera op
         CustomOperator op = Application().GetFactory().CreateObject(L"alembic_camera");
         op.AddOutputPort(camera.GetActivePrimitive().GetRef());
         op.AddInputPort(camera.GetActivePrimitive().GetRef());
         op.Connect();
         ops.Add(op.GetRef());
         op.PutParameterValue(L"path",filename);
         op.PutParameterValue(L"identifier",CString(objects[i].getFullName().c_str()));
      }
      else if(Alembic::AbcGeom::IPolyMesh::matches(objects[i].getMetaData()))
      {
         Alembic::Abc::IObject parent = objects[i].getParent();
         std::string name = objects[i].getName();
         if(Alembic::AbcGeom::IXform::matches(parent.getMetaData()))
            name = parent.getName();

         // let's create a mesh
         Alembic::AbcGeom::IPolyMesh meshIObject(objects[i],Alembic::Abc::kWrapExisting);
         Alembic::AbcGeom::IPolyMeshSchema meshSchema = meshIObject.getSchema();
         Alembic::AbcGeom::IPolyMeshSchema::Sample meshSample;
         meshSchema.get(meshSample);

         // prepare the mesh
         Alembic::Abc::P3fArraySamplePtr meshPos = meshSample.getPositions();
         Alembic::Abc::Int32ArraySamplePtr meshFaceCount = meshSample.getFaceCounts();
         Alembic::Abc::Int32ArraySamplePtr meshFaceIndices = meshSample.getFaceIndices();

         CVector3Array pos((LONG)meshPos->size());
         CLongArray polies((LONG)(meshFaceCount->size() + meshFaceIndices->size()));
         for(size_t j=0;j<meshPos->size();j++)
            pos[(LONG)j].Set((*meshPos)[j].x,(*meshPos)[j].y,(*meshPos)[j].z);
         LONG offset1 = 0, offset2 = 0;
         for(size_t j=0;j<meshFaceCount->size();j++)
         {
            polies[offset1++] = (*meshFaceCount)[j];
            offset2 += (*meshFaceCount)[j];
            for(size_t k=0;k<(*meshFaceCount)[j];k++)
            {
               polies[offset1++] = (*meshFaceIndices)[(size_t)offset2 - 1 - k];
            }
         }

         X3DObject meshObj;
         Application().GetActiveSceneRoot().AddPolygonMesh(pos,polies,name.c_str(),meshObj);

         // let's setup the xform op
         if(Alembic::AbcGeom::IXform::matches(parent.getMetaData()))
         {
            CustomOperator op = Application().GetFactory().CreateObject(L"alembic_xform");
            op.AddOutputPort(meshObj.GetKinematics().GetGlobal().GetRef());
            op.AddInputPort(meshObj.GetKinematics().GetGlobal().GetRef());
            op.Connect();
            ops.Add(op.GetRef());
            op.PutParameterValue(L"path",filename);
            op.PutParameterValue(L"identifier",CString(parent.getFullName().c_str()));
         }
         
         // let's setup the camera op
         CustomOperator op = Application().GetFactory().CreateObject(L"alembic_polymesh");
         op.AddOutputPort(meshObj.GetActivePrimitive().GetRef());
         op.AddInputPort(meshObj.GetActivePrimitive().GetRef());
         op.Connect();
         ops.Add(op.GetRef());
         op.PutParameterValue(L"path",filename);
         op.PutParameterValue(L"identifier",CString(objects[i].getFullName().c_str()));
      }
   }

   // check if we have any ops, if so let's set them up
   if(ops.GetCount() > 0)
      addArchive(archive);
   else
      delete(archive);

   return CStatus::OK;
}

SICALLBACK alembic_MenuExport_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Menu oMenu;
	oMenu = ctxt.GetSource();
	MenuItem oNewItem;
	oMenu.AddCommandItem(L"Alembic 1.0",L"alembic_export",oNewItem);
	return CStatus::OK;
}

SICALLBACK alembic_MenuImport_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Menu oMenu;
	oMenu = ctxt.GetSource();
	MenuItem oNewItem;
	oMenu.AddCommandItem(L"Alembic 1.0",L"alembic_import",oNewItem);
	return CStatus::OK;
}

