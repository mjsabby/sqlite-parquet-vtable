cmake -G "Unix Makefiles" -DARROW_BOOST_USE_SHARED=OFF -DARROW_BUILD_STATIC=ON -DARROW_DATASET=ON -DARROW_MIMALLOC=ON -DARROW_PARQUET=ON -DARROW_WITH_SNAPPY=ON -DPARQUET_REQUIRE_ENCRYPTION=OFF -DCMAKE_BUILD_TYPE="RELWITHDEBINFO" -DBOOST_ROOT=vendor/boost_1_75_0 vendor/arrow-apache-arrow-3.0.0/cpp -Boutput
cd output
make -j$(grep processor /proc/cpuinfo | tail -n 1 | awk '{print $3}')
cd ..
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RELWITHDEBINFO" src -Boutput2
cd output2
make -j$(grep processor /proc/cpuinfo | tail -n 1 | awk '{print $3}')
cd ..
g++ -o libParquetSQLite.so dll/dllmain.cpp -std=c++11 -Wl,--no-undefined -Wno-invalid-noreturn -g -fPIC -shared -O2 -Isrc -Lsrc -Loutput/relwithdebinfo -Loutput2 -Loutput/thrift_ep-install/lib -Loutput/snappy_ep/src/snappy_ep-install/lib -Loutput/mimalloc_ep/src/mimalloc_ep/lib/mimalloc-1.6 -DNDEBUG -Wl,--start-group -Wl,-Bstatic -larrow -lparquetsqlite -lparquet -larrow_bundled_dependencies -Wl,-Bdynamic -lpthread -ldl -Wl,--end-group -Wl,--as-needed && objcopy --only-keep-debug libParquetSQLite.so libParquetSQLite.dbg && objcopy --strip-debug libParquetSQLite.so
