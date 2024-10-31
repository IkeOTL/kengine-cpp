// format everything
// windows cmd: 
`for /r ./engine/src/ %t in (*.cpp) do clang-format -i %t`
`for /r ./engine/include/public/kengine/ %t in (*.hpp) do clang-format -i %t`
`for /r ./examples/basic/src %t in (*.cpp *.hpp) do clang-format -i %t`