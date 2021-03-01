cmake -G "Visual Studio 16 2019" -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/GL /MT /O2" -DARROW_BOOST_USE_SHARED=OFF -DARROW_BUILD_STATIC=ON -DARROW_DATASET=ON -DARROW_MIMALLOC=ON -DARROW_PARQUET=ON -DARROW_WITH_SNAPPY=ON -DPARQUET_REQUIRE_ENCRYPTION=OFF -DCMAKE_BUILD_TYPE="RELWITHDEBINFO" -DBOOST_ROOT=vendor/boost_1_75_0 vendor/arrow-apache-arrow-3.0.0/cpp -Boutput
msbuild output\src\parquet\parquet_static.vcxproj /p:Configuration=RelWithDebInfo /p:Platform=x64
cmake -G "Visual Studio 16 2019" src -Boutput2
msbuild output2\parquetsqlite.vcxproj /p:Configuration=RelWithDebInfo /p:Platform=x64
msbuild dll\ParquetSQLite.vcxproj /p:Configuration=Release /p:Platform=x64
copy dll\x64\Release\ParquetSQLite.dll ParquetSQLite.dll
copy dll\x64\Release\ParquetSQLite.pdb ParquetSQLite.pdb