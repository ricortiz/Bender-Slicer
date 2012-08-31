/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLModelNode - MRML node to represent a 3D surface model.
/// 
/// Model nodes describe polygonal data.  Models 
/// are assumed to have been constructed with the orientation and voxel 
/// dimensions of the original segmented volume.

#ifndef __vtkMRMLModelNode_h
#define __vtkMRMLModelNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"
class vtkMRMLModelDisplayNode;
class vtkMRMLStorageNode;

// VTK includes
class vtkDataArray;
class vtkPolyData;

class VTK_MRML_EXPORT vtkMRMLModelNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLModelNode *New();
  vtkTypeMacro(vtkMRMLModelNode,vtkMRMLDisplayableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Model";};

 /// Description:
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  /// 
  /// Get associated model display MRML node
  vtkMRMLModelDisplayNode* GetModelDisplayNode();

  /// 
  /// Set and observe poly data for this model
  vtkGetObjectMacro(PolyData, vtkPolyData);
  virtual void SetAndObservePolyData(vtkPolyData *PolyData);

  /// PolyDataModifiedEvent is fired when PolyData is changed.
  /// While it is possible for the subclasses to fire PolyDataModifiedEvent
  /// without modifying the polydata, it is not recommended to do so as it
  /// doesn't mark the polydata as modified, which my result in an incorrect
  /// return value for GetModifiedSinceRead()
  /// \sa GetModifiedSinceRead()
  enum
    {
      PolyDataModifiedEvent = 17001
    };

  /// 
  /// add an array to the polydata's point/cell data
  void AddPointScalars(vtkDataArray *array);
  void AddCellScalars(vtkDataArray *array);
  /// 
  /// remove an array from the polydata's point/cell data
  void RemoveScalars(const char *scalarName);
  
  /// 
  /// Get the currently active Point/Cell array name, type =
  /// scalars, vectors, normals, tcoords, tensors, null checks all in that
  /// order for an active array. Returns an empty string if it can't find one.
  const char *GetActivePointScalarName(const char *type);
  const char *GetActiveCellScalarName(const char *type);
  
  /// 
  /// Set the active poly data Point/Cell scalar array, checks for the
  /// scalarName as being a valid Point/Cell array, and then will set it to be the active
  /// attribute type as designated by typeName (scalars if null or
  /// empty). typeName is one of the valid strings as returned from
  /// vtkDataSetAttributes::GetAttributeTypeAsString, SetActiveScalars converts
  /// it to an integer type to pass onto the Point/Cell methods
  /// Also updates the display node's active scalars
  int SetActiveScalars(const char *scalarName, const char *typeName);
  int SetActivePointScalars(const char *scalarName, int attributeType);
  int SetActiveCellScalars(const char *scalarName, int attributeType);
  


  /// 
  /// Take scalar fields and composite them into a new one.
  /// New array will have values from the background array where the overlay is
  /// +/- if showOverlayPositive/Negative are 0.
  /// overlayMin and Max are used to adjust the color transfer function points,
  /// both should be positive, as they are mirrored around 0. -Min to Min gives the gap
  /// where the curvature will show through.
  /// New array name is backgroundName+overlayName
  /// Returns 1 on success, 0 on failure.
  /// Based on code from K. Teich, MGH
  int CompositeScalars(const char* backgroundName, const char* overlayName, float overlayMin, float overlayMax, int showOverlayPositive, int showOverlayNegative, int reverseOverlay);

  /// Get bounding box in global RAS the form (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual void GetRASBounds(double bounds[6]);

  virtual bool CanApplyNonLinearTransforms()const;
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Reimplemented to take into account the modified time of the polydata.
  /// Returns true if the node (default behavior) or the polydata are modified
  /// since read/written.
  /// Note: The MTime of the polydata is used to know if it has been modified.
  /// So if you invoke PolyDataModifiedEvent without calling Modified() on the
  /// polydata, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  virtual bool GetModifiedSinceRead();

protected:
  vtkMRMLModelNode();
  ~vtkMRMLModelNode();
  vtkMRMLModelNode(const vtkMRMLModelNode&);
  void operator=(const vtkMRMLModelNode&);

  /// Called when a display node is added. Propagate the polydata
  /// to the new display node.
  virtual void OnDisplayNodeAdded(vtkMRMLDisplayNode *dnode);

  virtual void SetPolyData(vtkPolyData* polyData);

  /// Data
  vtkPolyData *PolyData;
};

#endif
