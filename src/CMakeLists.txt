cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(PMDA_NAME "qpid")

project(pmda${PMDA_NAME})

# Add a pmdaqpid target.
add_executable(
    ${PROJECT_NAME}
    ${PROJECT_NAME}.cpp
    QpidLogger.cpp
)

# Detect which QMF libraries are available.
find_library(HAVE_QMF1 qmfconsole)
find_library(HAVE_QMF2 qmf2)
if (HAVE_QMF1 OR HAVE_QMF2)
    message(STATUS "Found the following QMF libraries:")
    if (HAVE_QMF1)
        message(STATUS "  QMFv1 (${HAVE_QMF1})")
    endif (HAVE_QMF1)
    if (HAVE_QMF2)
        message(STATUS "  QMFv2 (${HAVE_QMF2})")
    endif (HAVE_QMF2)
else ()
    message(FATAL_ERROR "Unable to find any QMF libraries.")
endif ()

# If QMF1, include the QMF1 source.
if (HAVE_QMF1)
    # add #define
    add_library(
        ${PROJECT_NAME}-qmf1 STATIC
        qmf1/ConsoleListener.cpp
        qmf1/ConsoleLogger.cpp
        qmf1/ConsoleUtils.cpp
        qmf1/QpidPmdaQmf1.cpp
    )
    target_link_libraries(
        ${PROJECT_NAME}
        ${PROJECT_NAME}-qmf1
        qmfconsole
        qpidclient
        qpidcommon
    )
endif ()

# If QMF2, include the QMF2 source.
if (HAVE_QMF2)
    message(STATUS "QMF2 support not implemented yet.")
endif ()

# Add Boost to the build.
find_package(Boost COMPONENTS program_options system REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# Add PCP libraries to the build.
target_link_libraries(${PROJECT_NAME} pcp pcp_pmda)

# Detect the PCP environment.
find_program(PCP_PMCONFIG_EXECUTABLE NAMES pmconfig)
if (PCP_PMCONFIG_EXECUTABLE)
    message(STATUS "Found pmconfig: ${PCP_PMCONFIG_EXECUTABLE}")
    execute_process(
        COMMAND ${PCP_PMCONFIG_EXECUTABLE} PCP_BINADM_DIR PCP_PMDAS_DIR
        OUTPUT_VARIABLE PCP_PMCONFIG_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    STRING(REPLACE "\n" ";" PCP_PMCONFIG_OUTPUT ${PCP_PMCONFIG_OUTPUT})
    foreach (PCP_PMCONFIG_LINE ${PCP_PMCONFIG_OUTPUT})
        string(REGEX REPLACE "=.*$" "" PCP_PMCONFIG_NAME  ${PCP_PMCONFIG_LINE})
        string(REGEX REPLACE "^[A-Z_]+=" "" PCP_PMCONFIG_VALUE ${PCP_PMCONFIG_LINE})
        set(${PCP_PMCONFIG_NAME} ${PCP_PMCONFIG_VALUE})
        message(STATUS "  ${PCP_PMCONFIG_NAME}: ${${PCP_PMCONFIG_NAME}}")
    endforeach ()
endif ()
find_program(PCP_NEWHELP_EXECUTABLE NAMES newhelp HINTS ${PCP_BINADM_DIR})
if (PCP_NEWHELP_EXECUTABLE)
    message(STATUS "Found newhelp: ${PCP_NEWHELP_EXECUTABLE}")
endif ()

# Create an install target, but only if the PCP_PMDA_DIR directory was detected.
configure_file(Install.in ${CMAKE_CURRENT_SOURCE_DIR}/Install)
configure_file(Remove.in  ${CMAKE_CURRENT_SOURCE_DIR}/Remove)
if (PCP_PMDAS_DIR)
    # Install the PMDA binary.
    install(
        TARGETS ${PROJECT_NAME} DESTINATION ${PCP_PMDAS_DIR}/${PMDA_NAME}
    )
    # Export the PMDA's support files (domain, help, pmns, etc).
    install(
        CODE "
            message(\"-- Exporting: \$ENV{DESTDIR}${PCP_PMDAS_DIR}/${PMDA_NAME}/{domain.h,help,pmns,root}\")
            execute_process(
                COMMAND \$ENV{DESTDIR}${PCP_PMDAS_DIR}/${PMDA_NAME}/${PROJECT_NAME} --export-all
                WORKING_DIRECTORY \$ENV{DESTDIR}${DESTDIR}${PCP_PMDAS_DIR}/${PMDA_NAME}
                RESULT_VARIABLE export_result
            )
            if (NOT \${export_result} EQUAL 0)
                message(FATAL_ERROR \"Failed to export PMDA support files: \${export_result}\")
            endif ()
            if (EXISTS ${PCP_NEWHELP_EXECUTABLE})
                message(\"-- Generating: \$ENV{DESTDIR}${PCP_PMDAS_DIR}/${PMDA_NAME}/help.{dir,pag}\")
                execute_process(
                    COMMAND ${PCP_NEWHELP_EXECUTABLE} -n root help
                    WORKING_DIRECTORY \$ENV{DESTDIR}${DESTDIR}${PCP_PMDAS_DIR}/${PMDA_NAME}
                    RESULT_VARIABLE export_result
                )
                if (NOT \${export_result} EQUAL 0)
                    message(FATAL_ERROR \"Failed to generate help database: \${export_result}\")
                endif ()
            endif ()
        "
    )
    # Install the PMDA registration scripts.
    install(
        FILES Install Remove DESTINATION ${PCP_PMDAS_DIR}/${PMDA_NAME}
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )
endif (PCP_PMDAS_DIR)

# Enable (and stop on) compiler warnings.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wall   HAVE_WALL)
check_cxx_compiler_flag(-Werror HAVE_WERROR)
check_cxx_compiler_flag(-Wextra HAVE_WEXTRA)
if (HAVE_WALL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if (HAVE_WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()
if (HAVE_WEXTRA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()
