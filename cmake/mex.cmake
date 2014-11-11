###########################################################################
#  cmake/mex.cmake
#  --------------------
#
#  Copyright (c) 2011, Edward T. Kaszubski ( ekaszubski@gmail.com )
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following disclaimer
#    in the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of usc-ros-pkg nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
###########################################################################

macro( include_mex )
	message( STATUS "----" )
	message( STATUS "Reading MEX options..." )

	message( STATUS "Reading environment variables..." )
	set( MATLAB_DIR $ENV{MATLAB_DIR} ) #/usr/local/MATLAB/R2011a )
	message( STATUS "MATLAB_DIR set to ${MATLAB_DIR}" )
	set( OS $ENV{OS} ) #glnx )
	message( STATUS "OS set to ${OS}" )
	set( ARCH $ENV{ARCH} ) #a64 )
	message( STATUS "ARCH set to ${ARCH}" )
	set( MEX_EXT $ENV{MEX_EXT} ) #a64
	message( STATUS "MEX_EXT set to ${MEX_EXT}" )
	message( STATUS "Done reading environment variables." )
	#set( INCLUDE_SIMULINK $ENV{INCLUDE_SIMULINK} ) #0 )

	message( STATUS "Setting flags..." )
	set( MEX_DEFINITIONS -DMATLAB_MEX_FILE -ansi -D_GNU_SOURCE -fPIC -fno-omit-frame-pointer -pthread -DMX_COMPAT_32 -O -DNDEBUG -Wl,--version-script,${MATLAB_DIR}/extern/lib/${OS}${ARCH}/mexFunction.map -Wl,--no-undefined -std=c++0x )
	message( STATUS "MEX_DEFINITIONS set to ${MEX_DEFINITIONS}" )
	set( MEX_LIBRARIES mx mex mat m )
	message( STATUS "MEX_LIBRARIES set to ${MEX_LIBRARIES}" )
	set( MEX_LIBRARY_DIRS ${MATLAB_DIR}/bin/${OS}${ARCH} )
	message( STATUS "MEX_LIBRARY_DIRS set to ${MEX_LIBRARY_DIRS}" )
	set( MEX_INCLUDE_DIRS ${MATLAB_DIR}/extern/include ${MATLAB_DIR}/simulink/include )
	message( STATUS "MEX_INCLUDE_DIRS set to ${MEX_INCLUDE_DIRS}" )
	message( STATUS "Done setting flags." )
	message( STATUS "MEX options read." )
	message( STATUS "----" )
	message( STATUS "Importing MEX flags..." )
	include_directories(${MEX_INCLUDE_DIRS})
	link_directories(${MEX_LIBRARY_DIRS})
	add_definitions(${MEX_DEFINITIONS})
	message( STATUS "Done importing MEX flags." )
	message( STATUS "----" )
endmacro( include_mex )

macro( add_mex lib )
  add_library( ${lib} SHARED ${ARGV} )
  target_link_libraries( ${lib} ${MEX_LIBRARIES} )
  add_custom_command(
    TARGET ${lib}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LIBRARY_OUTPUT_PATH}/lib${lib}.so ${PROJECT_SOURCE_DIR}/mex/${lib}.mex${MEX_EXT} )
endmacro( add_mex )

macro( rosbuild_add_mex lib )
  rosbuild_add_library( ${lib} ${ARGV} )
  target_link_libraries( ${lib} ${MATLAB_LIBS} )
  add_custom_command(
    TARGET ${lib}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LIBRARY_OUTPUT_PATH}/lib${lib}.so ${PROJECT_SOURCE_DIR}/mex/${lib}.mex${MEX_EXT} )
endmacro( rosbuild_add_mex )
