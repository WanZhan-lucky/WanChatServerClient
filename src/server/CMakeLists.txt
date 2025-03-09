#定义SRC_LIST变量，包含该目录下所有的源文件 , .表示当前目录下
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODLE_LIST)
aux_source_directory(./redis REDIS_LIST)

#指定可生成文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODLE_LIST} ${REDIS_LIST})
#可执行文件链接依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)