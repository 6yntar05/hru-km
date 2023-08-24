#!/bin/sh

sed "s/\${uname -r}/$(uname -r)/g" c_cpp_properties_proto.json > c_cpp_properties.json