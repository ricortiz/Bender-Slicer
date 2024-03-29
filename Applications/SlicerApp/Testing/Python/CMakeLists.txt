
#
# Check if '--python-code' / 'slicer.testing' works as expected
#

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest1
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerApp${SlicerApp_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "exit(slicer.util.EXIT_SUCCESS)"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest2
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerApp${SlicerApp_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "exit(slicer.util.EXIT_FAILURE)"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest2 PROPERTIES WILL_FAIL TRUE)

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest3
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerApp${SlicerApp_EXE_SUFFIX}>
  --ignore-slicerrc --testing --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "import slicer.testing; slicer.testing.exitSuccess()"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest4
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerApp${SlicerApp_EXE_SUFFIX}>
  --ignore-slicerrc --testing --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "import slicer.testing; slicer.testing.exitFailure('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest4 PROPERTIES WILL_FAIL TRUE)

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest5
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerApp${SlicerApp_EXE_SUFFIX}>
  --ignore-slicerrc --testing --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "raise Exception('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest5 PROPERTIES WILL_FAIL TRUE)

#
# Check if '--python-scripts' / 'testing' works as expected
#

slicer_add_python_test(
  SCRIPT SlicerTestingExitSuccessTest.py
  SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_test(
  SCRIPT SlicerTestingExitFailureTest.py
  SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_
  )
set_tests_properties(py_nomainwindow_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

slicer_add_python_test(SCRIPT SlicerTestingExitSuccessTest.py)

slicer_add_python_test(SCRIPT SlicerTestingExitFailureTest.py)
set_tests_properties(py_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)


#
# Check if 'slicer.testing.runUnitTest()' works as expected
#

slicer_add_python_unittest(
  SCRIPT SlicerUnitTestTest.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT SlicerUnitTestWithErrorsTest.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )
set_tests_properties(py_nomainwindow_SlicerUnitTestWithErrorsTest PROPERTIES WILL_FAIL TRUE)

#
# Exercise different Slicer command line option and check that no warnings are displated.
#
macro(nowarning_test test_name)
  add_test(
    NAME py_nowarning_${test_name}Test
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerApp${SlicerApp_EXE_SUFFIX}>
    ${ARGN} --ignore-slicerrc --testing --python-code "exit(slicer.util.EXIT_SUCCESS)"
    )
  set(fail_regexp)
  list(APPEND fail_regexp "Failed to obtain reference to")
  list(APPEND fail_regexp "Exception AttributeError: .* instance has no attribute")
  set_tests_properties(py_nowarning_${test_name}Test PROPERTIES FAIL_REGULAR_EXPRESSION "${fail_regexp}")
endmacro()
nowarning_test(mainwindow --no-splash)
nowarning_test(mainwindow_python_interactor --show-python-interactor)
nowarning_test(mainwindow_nocli --disable-cli-modules)
nowarning_test(mainwindow_noloadable --disable-loadable-modules)
nowarning_test(mainwindow_noscripted --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nocli_noloadable --disable-cli-modules --disable-loadable-modules)
nowarning_test(mainwindow_nocli_noscripted --disable-cli-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_noloadable_noscripted --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nocli_noloadable_noscripted --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow --no-main-window)
nowarning_test(nomainwindow_python_interactor --no-main-window --show-python-interactor)
nowarning_test(nomainwindow_nocli --no-main-window --disable-cli-modules)
nowarning_test(nomainwindow_noloadable --no-main-window --disable-loadable-modules)
nowarning_test(nomainwindow_noscripted --no-main-window --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nocli_noloadable --no-main-window --disable-cli-modules --disable-loadable-modules)
nowarning_test(nomainwindow_nocli_noscripted --no-main-window --disable-cli-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_noloadable_noscripted --no-main-window --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nocli_noloadable_noscripted --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_ignoreslicerrc --no-main-window --ignore-slicerrc)


#
# Application tests
#
slicer_add_python_unittest(
  SCRIPT SlicerSceneObserverTest.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )

if(VTK_DEBUG_LEAKS AND Slicer_HAS_CONSOLE_IO_SUPPORT)
  set(testname MRMLCreateNodeByClassWithoutSetReferenceCount)
  slicer_add_python_test(
    SCRIPT ${testname}.py
    SCRIPT_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
    TESTNAME_PREFIX nomainwindow_
    )
  set_tests_properties(py_nomainwindow_${testname}
    PROPERTIES PASS_REGULAR_EXPRESSION "instances? still around"
    )
endif()

set(testname MRMLCreateNodeByClassWithSetReferenceCountMinusOne)
slicer_add_python_test(
  SCRIPT ${testname}.py
  SCRIPT_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )

set(testname MRMLCreateNodeByClassWithSetReferenceCountToOne)
slicer_add_python_test(
  SCRIPT ${testname}.py
  SCRIPT_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )
set_tests_properties(py_nomainwindow_${testname} PROPERTIES WILL_FAIL TRUE)

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/tests/test_saferef.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )

#
# SelfTests
# see http://wiki.slicer.org/slicerWiki/index.php/Documentation/Nightly/Developers/Tutorials/SelfTestModule
#

if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)

  # add as unit test for use at build/test time
  slicer_add_python_unittest(SCRIPT AtlasTests.py)
  slicer_add_python_unittest(SCRIPT AbdominalAtlasTest.py)
  slicer_add_python_unittest(SCRIPT KneeAtlasTest.py)
  slicer_add_python_unittest(SCRIPT fiber_visibility_crash2438.py)
  slicer_add_python_unittest(SCRIPT labelToggleBug2049.py)
  slicer_add_python_unittest(SCRIPT sceneImport2428.py)
  slicer_add_python_unittest(SCRIPT SlicerMRBTest.py)
  slicer_add_python_unittest(SCRIPT Slicer4Minute.py)
  slicer_add_python_unittest(SCRIPT Charting.py)
  slicer_add_python_unittest(SCRIPT RSNA2012Vis.py)
  slicer_add_python_unittest(SCRIPT RSNA2012Quant.py)

  # add as hidden module for use at run time
  set(KIT_PYTHON_SCRIPTS
    AtlasTests.py
    fiber_visibility_crash2438.py
    labelToggleBug2049.py
    sceneImport2428.py
    SlicerMRBTest.py
    Slicer4Minute.py
    Charting.py
    RSNA2012Vis.py
    RSNA2012Quant.py
    )

  set(KIT_PYTHON_RESOURCES
    )

  ctkMacroCompilePythonScript(
    TARGET_NAME ApplicationSelfTests
    SCRIPTS "${KIT_PYTHON_SCRIPTS}"
    RESOURCES "${KIT_PYTHON_RESOURCES}"
    DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
    INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
    NO_INSTALL_SUBDIR
    )

endif()

#
# Tutorial tests
#

if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)

  configure_file(
    slicerappdatapaths.py.in
    ${Slicer_BINARY_DIR}/bin/Python/slicer/slicerappdatapaths.py
    @ONLY
    )

  set(tests_using_qttesting)

  if(Slicer_BUILD_CLI)
    slicer_add_python_test(
      SCRIPT NeurosurgicalPlanningTutorial.py
      SLICER_ARGS --launcher-no-splash --qt-testing
      )

    slicer_add_python_test(
      SCRIPT DiffusionTensorImagingTutorial.py
      SLICER_ARGS --launcher-no-splash --qt-testing
      )

    list(APPEND tests_using_qttesting
      py_DiffusionTensorImagingTutorial
      py_NeurosurgicalPlanningTutorial
      )
  endif()


  set_tests_properties(
    ${tests_using_qttesting}
    PROPERTIES RUN_SERIAL ON
    )
endif()
