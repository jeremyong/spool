find_package(SQLite3 REQUIRED)
add_executable(spooler
    Database.cpp
    Generator.cpp
    Main.cpp
    Origins.cpp
    Parser.cpp
    Statement.cpp
    Strings.cpp
    )
target_link_libraries(spooler PUBLIC sqlite3)
