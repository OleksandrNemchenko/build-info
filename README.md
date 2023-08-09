
This is single C++14 file that can be built by the commands listed below:
- gcc: `g++ -std=c++14 -o buildInfo buildInfo.cpp` where `buildInfo` is the binary name.
- Visual Studio: start Command Prompt with the required environment and execute `cl /EHsc /std:c++14 buildInfo.cpp /Fe:buildInfo.exe` commadn where `buildInfo.exe` is the required binary name. Also `buildInfo.obj` file will be created and could be deleted after build.
Command line:

`
buildInfo  --template-file <template_file> --build-time <build_time_template>
    --result-file <result_file> --git-commit <git_commit_template>
    --git-time <git_time_template> --build-number-pattern <build_number_pattern>
    --build-number-file <build_number_file> [--dont-increment-build-number]
`

Arguments:
- `--template-file` : source file name with template
- `--result-file`: destination file with the source file replacements. May be ignored for the git push command.
- `--build-time` : current build time replacing `<build_time_template>` text.
- `--git-commit` : last git commit number.
- `--git-time` : last git time.
- `--build-number-file`: text file with the build number. If this file does not exist and `--dont-increment-build-number` attribute is absent, file will be created.
- `--build-number-pattern`: replacement template for the build number.
- `--dont-increnemt-build-number`: if this attribute is specified, build number won't be incremented. Otherwise it is incremented.

There are example files:
- `buildInfoPattern.h` is an example of the source file
- `buildNumber.tx  t` is an auto generated text file with the current build number
- `buildInfoResult.h` is the result file genereated from pattern

