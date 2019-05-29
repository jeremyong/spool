set(SPOOL_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
set(SPOOL_MACRO "SP")

function(spool TARG)
    if (${ARGV1})
        set(SPOOL ${ARGV1})
    else()
        set(SPOOL default_spool)
    endif()
    set(SPOOL_GENERATOR ${SPOOL}_GENERATOR)
    set(SPOOL_DB ${CMAKE_BINARY_DIR}/${SPOOL}.db)
    set(SPOOL_SOURCE ${CMAKE_BINARY_DIR}/${SPOOL}.cpp)
    if (NOT TARGET ${SPOOL})
        define_property(TARGET PROPERTY SPOOL_FILE_COUNTER
            BRIEF_DOCS "Spool file counter"
            FULL_DOCS "This is tracked per spool to assign unique ids to each source file")

        # Initialize SQLite database
        message("Initializing database for spool ${SPOOL}: ${SPOOL_DB}")

        execute_process(COMMAND
            sqlite3 ${SPOOL}.db ".read ${SPOOL_PROJECT_DIR}/sql/spool.sql"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )


        add_library(${SPOOL} ${SPOOL_SOURCE})
        set_source_files_properties(${SPOOL_SOURCE} PROPERTIES GENERATED TRUE)
        set_target_properties(${SPOOL} PROPERTIES SPOOL_FILE_COUNTER 0)
        add_custom_target(${SPOOL_GENERATOR}
            $<TARGET_FILE:spooler> generate ${SPOOL}.db ${SPOOL}.cpp
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
        add_dependencies(${SPOOL} ${SPOOL_GENERATOR})
    endif()

    target_link_libraries(${TARG} PUBLIC ${SPOOL} spool)
    get_target_property(TARG_SOURCES ${TARG} SOURCES)
    get_target_property(TARG_SOURCE_DIR ${TARG} SOURCE_DIR)
    
    get_target_property(SPOOL_FILE_ID ${SPOOL} SPOOL_FILE_COUNTER)

    foreach(TARG_SOURCE ${TARG_SOURCES})
        # Add a monotonically increasing compile definition for each source file in a spool
        set_source_files_properties(${TARG_SOURCE_DIR}/${TARG_SOURCE}
            PROPERTIES COMPILE_DEFINITIONS SPOOL_ID=${SPOOL_FILE_ID})

        execute_process(COMMAND
            sqlite3 ${SPOOL}.db "INSERT INTO sources (path_id, path) VALUES (${SPOOL_FILE_ID}, \"${TARG_SOURCE_DIR}/${TARG_SOURCE}\")"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )

        # Add a dependency to the final spool target
        add_custom_command(TARGET ${SPOOL_GENERATOR}
            PRE_BUILD
            COMMAND $<TARGET_FILE:spooler> analyze ${SPOOL}.db ${TARG_SOURCE_DIR}/${TARG_SOURCE} ${SPOOL_MACRO} ${SPOOL_FILE_ID}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )

        MATH(EXPR SPOOL_FILE_ID "${SPOOL_FILE_ID} + 1")
    endforeach()

    set_target_properties(${SPOOL} PROPERTIES SPOOL_FILE_COUNTER ${SPOOL_FILE_ID})
    message(${TARG_SOURCE_DIR})

endfunction()
