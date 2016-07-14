#Find libhiredis
#
#	HIREDIS_INCLUDE_DIRS		-	where to find hiredis/hiredis.h
#	HIREDIS_LIBRARIES		-	list of libraries when using libhiredis
#	HIREDIS_FOUND			-	true if libhiredis found
#	HIREDIS_VERSION_STRING		-	the version of the libhiredis found
#

find_path(HIREDIS_INCLUDE_DIR NAMES hiredis/hiredis.h)

find_library(HIREDIS_LIBRARY NAMES
	hiredis
)

if(HIREDIS_INCLUDE_DIR)
	if(EXISTS "${HIREDIS_INCLUDE_DIR}/hiredis/hiredis.h")
		set(hiredis_ver_tmp_major 0)
		set(hiredis_ver_tmp_minor 0)
		set(hiredis_ver_tmp_patch 0)
		file(STRINGS "${HIREDIS_INCLUDE_DIR}/hiredis/hiredis.h" hiredis_version_nums REGEX "^#define[\t ]+HIREDIS_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+")
		foreach(hiredis_version_def IN LISTS hiredis_version_nums)
			if ("${hiredis_version_def}" MATCHES "HIREDIS_MAJOR")
				string(REGEX REPLACE "^#define[\t ]+HIREDIS_MAJOR[\t ]+([0-9]+).*" "\\1" hiredis_ver_tmp_major "${hiredis_version_def}")
			elseif ("${hiredis_version_def}" MATCHES "HIREDIS_MINOR")
				string(REGEX REPLACE "^#define[\t ]+HIREDIS_MINOR[\t ]+([0-9]+).*" "\\1" hiredis_ver_tmp_minor "${hiredis_version_def}")
			elseif ("${hiredis_version_def}" MATCHES "HIREDIS_PATCH")
				string(REGEX REPLACE "^#define[\t ]+HIREDIS_PATCH[\t ]+([0-9]+).*" "\\1" hiredis_ver_tmp_patch "${hiredis_version_def}")
			endif()
		endforeach()

		unset(hiredis_version_nums)
		unset(hiredis_version_def)
		set(HIREDIS_VERSION_STRING "${hiredis_ver_tmp_major}.${hiredis_ver_tmp_minor}.${hiredis_ver_tmp_patch}")
		unset(hiredis_ver_tmp_major)
		unset(hiredis_ver_tmp_minor)
		unset(hiredis_ver_tmp_patch)
	endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(hiredis
	REQUIRED_VARS HIREDIS_LIBRARY HIREDIS_INCLUDE_DIR
	VERSION_VAR HIREDIS_VERSION_STRING
)

if(HIREDIS_FOUND)
	set(HIREDIS_LIBRARIES ${HIREDIS_LIBRARY})
	set(HIREDIS_INCLUDE_DIRS ${HIREDIS_INCLUDE_DIR})
endif()

mark_as_advanced(
    HIREDIS_ROOT_DIR
    HIREDIS_INCLUDE_DIR
    HIREDIS_LIBRARY
)
