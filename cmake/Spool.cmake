set(SPOOL_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
set(SPOOL_MACRO "SP")

function(spool_file TARG TARG_SOURCE)
    if (ARGV2)
        set(SPOOL ${ARGV2})
    else()
        set(SPOOL default_spool)
    endif()
    set(SPOOL_DIR ${CMAKE_BINARY_DIR}/spool)
    set(SPOOL_TMP ${SPOOL}_TMP)
    set(SPOOL_DB ${SPOOL_DIR}/${SPOOL}.db)
    set(SPOOL_SOURCE ${SPOOL_DIR}/${SPOOL}.cpp)
    if (NOT TARGET ${SPOOL})
        define_property(TARGET PROPERTY SPOOL_FILE_COUNTER
            BRIEF_DOCS "Spool file counter"
            FULL_DOCS "This is tracked per spool to assign unique ids to each source file")

        # Initialize SQLite database

        add_custom_command(
            OUTPUT ${SPOOL_DB}
            COMMAND sqlite3 ${SPOOL}.db ".read ${SPOOL_PROJECT_DIR}/sql/spool.sql"
            WORKING_DIRECTORY ${SPOOL_DIR}
            COMMENT "Initializing database for spool ${SPOOL}: ${SPOOL_DB}"
            )

        file(MAKE_DIRECTORY ${SPOOL_DIR})
        file(TOUCH ${SPOOL_SOURCE})
        file(MAKE_DIRECTORY ${SPOOL_DIR}/${SPOOL_TMP})

        add_library(${SPOOL} ${SPOOL_SOURCE})
        set_target_properties(${SPOOL} PROPERTIES SPOOL_FILE_COUNTER 0)

        add_custom_command(
            OUTPUT ${SPOOL_SOURCE}
            COMMAND $<TARGET_FILE:spooler> generate ${SPOOL}.db ${SPOOL}.cpp
            WORKING_DIRECTORY ${SPOOL_DIR}
            DEPENDS ${SPOOL_DB}
            COMMENT "Populating ${SPOOL}.cpp with data from ${SPOOL}.db"
            )
        add_dependencies(${SPOOL} spooler)
    endif()

    target_link_libraries(${TARG} PUBLIC ${SPOOL} spool)
    get_target_property(TARG_SOURCE_DIR ${TARG} SOURCE_DIR)
    get_target_property(SPOOL_FILE_ID ${SPOOL} SPOOL_FILE_COUNTER)

    # Add a monotonically increasing compile definition for each source file in a spool
    set_source_files_properties(${TARG_SOURCE}
        PROPERTIES COMPILE_DEFINITIONS SPOOL_ID=${SPOOL_FILE_ID})

    message("Adding ${TARG_SOURCE} to spool ${SPOOL} (id: ${SPOOL_FILE_ID})")

    set(SPOOL_SENTINEL ${SPOOL_TMP}/${SPOOL}_${SPOOL_FILE_ID})

    if (SPOOL_FILE_ID GREATER 0)
        # TODO DB access is not transactional yet, so files are parsed one at a time
        math(EXPR LAST_FILE_ID "${SPOOL_FILE_ID} - 1")
        set(LAST_SENTINEL ${SPOOL_DIR}/${SPOOL_TMP}/${SPOOL}_${LAST_FILE_ID})
    endif()

    # Parse source file for spool-designated strings and extract them into the spool database
    add_custom_command(
        OUTPUT ${SPOOL_DIR}/${SPOOL_SENTINEL}
        COMMAND ${CMAKE_COMMAND} -E touch ${SPOOL_SENTINEL}
        COMMAND $<TARGET_FILE:spooler> analyze ${SPOOL}.db ${TARG_SOURCE_DIR}/${TARG_SOURCE} ${SPOOL_MACRO} ${SPOOL_FILE_ID}
        WORKING_DIRECTORY ${SPOOL_DIR}
        DEPENDS ${TARG_SOURCE_DIR}/${TARG_SOURCE} spooler ${LAST_SENTINEL}
        COMMENT "Running spooler on ${TARG_SOURCE}"
        )

    get_source_file_property(SPOOL_DEPS ${SPOOL_SOURCE} OBJECT_DEPENDS)
    if (SPOOL_DEPS STREQUAL "NOTFOUND")
        set(SPOOL_DEPS ${SPOOL_DIR}/${SPOOL_SENTINEL})
    else()
        list(APPEND SPOOL_DEPS ${SPOOL_DIR}/${SPOOL_SENTINEL})
    endif()
    set_source_files_properties(${SPOOL_SOURCE} PROPERTIES OBJECT_DEPENDS "${SPOOL_DEPS}")

    MATH(EXPR SPOOL_FILE_ID "${SPOOL_FILE_ID} + 1")
endfunction()

function(spool TARG)
    if (ARGV1)
        set(SPOOL ${ARGV1})
    else()
        set(SPOOL default_spool)
    endif()
    set(SPOOL_DIR ${CMAKE_BINARY_DIR}/spool)
    set(SPOOL_TMP ${SPOOL}_TMP)
    set(SPOOL_DB ${SPOOL_DIR}/${SPOOL}.db)
    set(SPOOL_SOURCE ${SPOOL_DIR}/${SPOOL}.cpp)
    if (NOT TARGET ${SPOOL})
        define_property(TARGET PROPERTY SPOOL_FILE_COUNTER
            BRIEF_DOCS "Spool file counter"
            FULL_DOCS "This is tracked per spool to assign unique ids to each source file")

        # Initialize SQLite database

        add_custom_command(
            OUTPUT ${SPOOL_DB}
            COMMAND sqlite3 ${SPOOL}.db ".read ${SPOOL_PROJECT_DIR}/sql/spool.sql"
            WORKING_DIRECTORY ${SPOOL_DIR}
            COMMENT "Initializing database for spool ${SPOOL}: ${SPOOL_DB}"
            )

        file(MAKE_DIRECTORY ${SPOOL_DIR})
        file(TOUCH ${SPOOL_SOURCE})
        file(MAKE_DIRECTORY ${SPOOL_DIR}/${SPOOL_TMP})

        add_library(${SPOOL} ${SPOOL_SOURCE})
        set_target_properties(${SPOOL} PROPERTIES SPOOL_FILE_COUNTER 0)

        add_custom_command(
            OUTPUT ${SPOOL_SOURCE}
            COMMAND $<TARGET_FILE:spooler> generate ${SPOOL}.db ${SPOOL}.cpp
            WORKING_DIRECTORY ${SPOOL_DIR}
            DEPENDS ${SPOOL_DB}
            COMMENT "Populating ${SPOOL}.cpp with data from ${SPOOL}.db"
            )
        add_dependencies(${SPOOL} spooler)
    endif()

    target_link_libraries(${TARG} PUBLIC ${SPOOL} spool)
    get_target_property(TARG_SOURCES ${TARG} SOURCES)
    get_target_property(TARG_SOURCE_DIR ${TARG} SOURCE_DIR)

    get_target_property(SPOOL_FILE_ID ${SPOOL} SPOOL_FILE_COUNTER)

    foreach(TARG_SOURCE ${TARG_SOURCES})
        # Add a monotonically increasing compile definition for each source file in a spool
        set_source_files_properties(${TARG_SOURCE_DIR}/${TARG_SOURCE}
            PROPERTIES COMPILE_DEFINITIONS SPOOL_ID=${SPOOL_FILE_ID})

        message("Adding ${TARG_SOURCE} to spool ${SPOOL} (id: ${SPOOL_FILE_ID})")

        set(SPOOL_SENTINEL ${SPOOL_TMP}/${SPOOL}_${SPOOL_FILE_ID})

        if (SPOOL_FILE_ID GREATER 0)
            # TODO DB access is not transactional yet, so files are parsed one at a time
            math(EXPR LAST_FILE_ID "${SPOOL_FILE_ID} - 1")
            set(LAST_SENTINEL ${SPOOL_DIR}/${SPOOL_TMP}/${SPOOL}_${LAST_FILE_ID})
        endif()

        # Parse source file for spool-designated strings and extract them into the spool database
        add_custom_command(
            OUTPUT ${SPOOL_DIR}/${SPOOL_SENTINEL}
            COMMAND ${CMAKE_COMMAND} -E touch ${SPOOL_SENTINEL}
            COMMAND $<TARGET_FILE:spooler> analyze ${SPOOL}.db ${TARG_SOURCE_DIR}/${TARG_SOURCE} ${SPOOL_MACRO} ${SPOOL_FILE_ID}
            WORKING_DIRECTORY ${SPOOL_DIR}
            DEPENDS ${TARG_SOURCE_DIR}/${TARG_SOURCE} spooler ${LAST_SENTINEL}
            COMMENT "Running spooler on ${TARG_SOURCE}"
            )

        get_source_file_property(SPOOL_DEPS ${SPOOL_SOURCE} OBJECT_DEPENDS)
        if (SPOOL_DEPS STREQUAL "NOTFOUND")
            set(SPOOL_DEPS ${SPOOL_DIR}/${SPOOL_SENTINEL})
        else()
            list(APPEND SPOOL_DEPS ${SPOOL_DIR}/${SPOOL_SENTINEL})
        endif()
        set_source_files_properties(${SPOOL_SOURCE} PROPERTIES OBJECT_DEPENDS "${SPOOL_DEPS}")

        MATH(EXPR SPOOL_FILE_ID "${SPOOL_FILE_ID} + 1")
    endforeach()

    set_target_properties(${SPOOL} PROPERTIES SPOOL_FILE_COUNTER ${SPOOL_FILE_ID})
endfunction()
