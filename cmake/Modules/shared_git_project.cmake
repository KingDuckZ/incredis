cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)

function (add_shared_git_project SUBMODULE_PATH)
	if (IS_ABSOLUTE "${SUBMODULE_PATH}")
		set(submod_path "${SUBMODULE_PATH}")
	else()
		set(submod_path "${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}")
	endif()

	if (NOT EXISTS "${submod_path}")
		message(FATAL_ERROR "Path \"${submod_path}\" doesn't exist")
	endif()
	if (NOT IS_DIRECTORY "${submod_path}")
		message(FATAL_ERROR "Path \"${submod_path}\" is not a valid directory")
	endif()

	if (NOT EXISTS "${submod_path}/.git")
		message(FATAL_ERROR ".git not found in \"${submod_path}\". Not a git submodule?")
	endif()
	if (IS_DIRECTORY "${submod_path}/.git")
		message(FATAL_ERROR "\"${submod_path}.git\" is a directory, not a file as expected. Not a git submodule?")
	endif()

	get_filename_component(proj_name_orig "${submod_path}" NAME)
	if ("${proj_name_orig}" STREQUAL "")
		message(FATAL_ERROR "Couldn't make up a name for given project in \"${submod_path}\"")
	endif()

	string(MAKE_C_IDENTIFIER "${proj_name_orig}" proj_name_c_id)
	string(TOUPPER ${proj_name_c_id} proj_name)

	get_property(shared_projects_list GLOBAL PROPERTY SHARED_PROJECTS_LIST)
	list(FIND shared_projects_list ${proj_name} found_index)
	if (${found_index} GREATER -1)
		#nothing to do, the variable is already set so the project must have been
		#included already
		return()
	endif()

	#Obtain the path to the working tree
	# see http://stackoverflow.com/questions/27379818/git-possible-to-use-same-submodule-working-copy-by-multiple-projects
	# git rev-parse --git-dir --show-toplevel
	execute_process(
		COMMAND git rev-parse --show-toplevel
		WORKING_DIRECTORY "${submod_path}"
		OUTPUT_VARIABLE matched_gitdir
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	#Make sure we got an absolute path
	if (IS_ABSOLUTE "${matched_gitdir}")
		set(reported_submodule_dir "${matched_gitdir}")
	else()
		file(RELATIVE_PATH reported_submodule_dir "${CMAKE_CURRENT_SOURCE_DIR}" "${submod_path}/${matched_gitdir}")
	endif()
	unset(matched_gitdir)

	#Check if submodule is a subdirectory of the current source dir
	file(RELATIVE_PATH reported_submodule_rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${reported_submodule_dir}")
	string(LENGTH "${reported_submodule_rel_path}" rel_path_len)
	if (${rel_path_len} GREATER 2)
		string(SUBSTRING "${reported_submodule_rel_path}" 0 3 first_bit)
		if ("../" STREQUAL "${first_bit}")
			set(is_out_of_dirtree ON)
		else()
			set(is_out_of_dirtree OFF)
		endif()
		unset(first_bit)
	else()
		set(is_out_of_dirtree OFF)
	endif()
	unset(rel_path_len)

	#Globally mark current submodule as handled
	set_property(GLOBAL APPEND PROPERTY SHARED_PROJECTS_LIST ${proj_name})

	set(shared_project_binary "${CMAKE_CURRENT_BINARY_DIR}/shared_projects/${proj_name_orig}")
	if (is_out_of_dirtree)
		#message(FATAL_ERROR "Would call add_subdirectory(\"${reported_submodule_dir}\" \"${shared_project_binary}\")")
		add_subdirectory("${reported_submodule_dir}" "${shared_project_binary}" ${ARGN})
	else()
		#message(FATAL_ERROR "Would call add_subdirectory(\"${reported_submodule_rel_path}\")")
		add_subdirectory("${reported_submodule_rel_path}" ${ARGN})
	endif()
endfunction()
