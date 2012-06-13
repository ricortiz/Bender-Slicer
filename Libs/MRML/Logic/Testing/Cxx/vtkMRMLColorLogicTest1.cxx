/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLLogic includes
#include "vtkMRMLColorLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkLookupTable.h>
#include <vtkTimerLog.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLColorLogicTest1(int , char * [] )
{
  // To load the freesurfer file, SLICER_HOME is requested
  //vtksys::SystemTools::PutEnv("SLICER_HOME=..." );
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkMRMLColorLogic* colorLogic = vtkMRMLColorLogic::New();
  colorLogic->SetDebug(1);

  vtkSmartPointer<vtkTimerLog> overallTimer = vtkSmartPointer<vtkTimerLog>::New();
  overallTimer->StartTimer();

  colorLogic->SetMRMLScene(scene);

  overallTimer->StopTimer();
  std::cout << "AddDefaultColorNodes: " << overallTimer->GetElapsedTime() << "s"
            << " " << 1. / overallTimer->GetElapsedTime() << "fps" << std::endl;
  overallTimer->StartTimer();

  vtkMRMLColorNode* colorNode = colorLogic->LoadColorFile("/Users/exxos/Work/Data/lut.ctbl");
  colorNode->Print(std::cout);
  double color[4];
  std::cout << ">> " << colorNode->GetLookupTable()->GetIndex(2.)
            << " " << colorNode->GetLookupTable()->GetTableValue(2)[0]
            << ", " << colorNode->GetLookupTable()->GetTableValue(2)[1]
            << ", " << std::endl;

  colorLogic->Delete();

  std::cout << "RemoveDefaultColorNodes: " << overallTimer->GetElapsedTime() << "s"
            << " " << 1. / overallTimer->GetElapsedTime() << "fps" << std::endl;

  return EXIT_SUCCESS;
}

