
# 定义了一个SRC_LIST变量 ，包含了当前文件夹下的所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST})

#可执行文件依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread)

