function(ossia_add_example TESTNAME TESTSRCS)
    add_executable(ossia_${TESTNAME}_example ${TESTSRCS})

    target_compile_options(ossia_${TESTNAME}_example PUBLIC ${OSSIA_COMPILE_OPTIONS})
    target_link_libraries(ossia_${TESTNAME}_example PUBLIC ossia)

    if(OSSIA_SANITIZE)
        sanitize_build(ossia_${TESTNAME}_example)
    endif()
    use_gold(ossia_${TESTNAME}_example)
endFunction()


if(OSSIA_DATAFLOW)
    ossia_add_example(editor_graph "${CMAKE_CURRENT_SOURCE_DIR}/Editor/example_graph.cpp")
    ossia_add_example(mixsines "${CMAKE_CURRENT_SOURCE_DIR}/Dataflow/MixSines.cpp")
    ossia_add_example(mixsinesaddr "${CMAKE_CURRENT_SOURCE_DIR}/Dataflow/MixSinesAddress.cpp")
endif()

if(OSSIA_EDITOR)
  ossia_add_example(editor_curve "${CMAKE_CURRENT_SOURCE_DIR}/Editor/curve.cpp")
  ossia_add_example(editor_expression "${CMAKE_CURRENT_SOURCE_DIR}/Editor/expression.cpp")
  ossia_add_example(editor_scenario "${CMAKE_CURRENT_SOURCE_DIR}/Editor/scenario.cpp")
  ossia_add_example(editor_state "${CMAKE_CURRENT_SOURCE_DIR}/Editor/state.cpp")
  ossia_add_example(editor_automation "${CMAKE_CURRENT_SOURCE_DIR}/Editor/automation.cpp")
endif()

if(OSSIA_PROTOCOL_MINUIT)
  ossia_add_example(network_minuit_exploration "${CMAKE_CURRENT_SOURCE_DIR}/Network/Minuit_exploration.cpp")
  ossia_add_example(network_minuit_publication "${CMAKE_CURRENT_SOURCE_DIR}/Network/Minuit_publication.cpp")
  ossia_add_example(network_minuit_publication_large "${CMAKE_CURRENT_SOURCE_DIR}/Network/Minuit_publication_large.cpp")
endif()

if(OSSIA_PROTOCOL_OSC)
  ossia_add_example(network_osc "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSC.cpp")
  ossia_add_example(network_osc_echo "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSC_echo.cpp")
  ossia_add_example(network_osc_send "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSC_send.cpp")
  ossia_add_example(network_rate_limiter "${CMAKE_CURRENT_SOURCE_DIR}/Network/RateLimiter.cpp")
endif()

if(OSSIA_PROTOCOL_OSCQUERY)
  ossia_add_example(network_oscquery_publication "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSCQuery_publication.cpp")
  ossia_add_example(network_oscquery_publication_simple "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSCQuery_publication_simple.cpp")
  ossia_add_example(network_oscquery_exploration "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSCQuery_exploration.cpp")
  ossia_add_example(network_oscquery_preset "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSCQuery_preset.cpp")
  ossia_add_example(network_oscquery_various "${CMAKE_CURRENT_SOURCE_DIR}/Network/OSCQuery_various.cpp")

  ossia_add_example(pd_controller "${CMAKE_CURRENT_SOURCE_DIR}/Network/PdController.cpp")
  if(OSSIA_PROTOCOL_LEAPMOTION)
    ossia_add_example(network_leapmotion_oscquery "${CMAKE_CURRENT_SOURCE_DIR}/Network/LeapToOscQuery.cpp")
  endif()

  if(OSSIA_QT)
    find_package(Qt5 COMPONENTS Widgets)
    ossia_add_example(qt_colors "${CMAKE_CURRENT_SOURCE_DIR}/Qt/Colors/main.cpp")
    target_link_libraries(ossia_qt_colors_example PRIVATE Qt5::Widgets)
  endif()
endif()

if(OSSIA_PROTOCOL_SERIAL)
  ossia_add_example(network_arduino "${CMAKE_CURRENT_SOURCE_DIR}/Network/Arduino.cpp")
endif()

#ossia_add_example(preset_device_example "${CMAKE_CURRENT_SOURCE_DIR}/Preset/DeviceExample.cpp")
#ossia_add_example(preset_parsing_example "${CMAKE_CURRENT_SOURCE_DIR}/Preset/ParsingExample.cpp")
ossia_add_example(preset_print_namespace "${CMAKE_CURRENT_SOURCE_DIR}/Preset/PrintNamespace.cpp")
ossia_add_example(export_namespace_to_json "${CMAKE_CURRENT_SOURCE_DIR}/Preset/Export_namespace_to_json.cpp")
ossia_add_example(logger "${CMAKE_CURRENT_SOURCE_DIR}/Common/Logger.cpp")

