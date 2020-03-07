# project(libssh-examples C CXX)

set(ssh_SRCS
  authentication.c
  knownhosts.c
  connect_ssh.c
)

add_executable(ssh-client ssh_client.c ${ssh_SRCS})
target_compile_options(ssh-client PRIVATE ${DEFAULT_C_COMPILE_FLAGS})
target_link_libraries(ssh-client ssh)
