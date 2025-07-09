add_executable(${PROJECT_NAME}_link link.c)
target_link_libraries(${PROJECT_NAME}_link ${PROJECT_NAME}_static)
target_include_directories(${PROJECT_NAME}_link
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
install(TARGETS ${PROJECT_NAME}_link)
add_executable(${PROJECT_NAME}_merge merge.c)
target_link_libraries(${PROJECT_NAME}_merge ${PROJECT_NAME}_static)
target_include_directories(${PROJECT_NAME}_merge
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
install(TARGETS ${PROJECT_NAME}_merge)
