set(TXN_POW_SOURCES txn_pow.cpp)
add_executable(txn_pow_test ${TXN_POW_SOURCES})
target_link_libraries(txn_pow_test crypto-static)
set_property(TARGET txn_pow_test PROPERTY OUTPUT_NAME "txn_pow_test")
