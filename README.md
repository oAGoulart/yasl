[![scriptwrapper](https://live.staticflickr.com/65535/50828244006_9abb85ac92_k.jpg)]()

![Build Status](https://dev.azure.com/agoulart/scriptwrapper/_apis/build/status/oAGoulart.scriptwrapper?branchName=master)
![Platform](https://img.shields.io/badge/platform-win--32%20%7C%20win--64-lightgrey)
[![License](https://img.shields.io/badge/license-MIT-informational.svg)](https://opensource.org/licenses/MIT)

A module to improve the development of modding scripts.  Currently not production ready.

A quick example of a detour before and after the function call:
```cpp
// function |int getPotato(int arg0)| at address 0x6050607 
memory::Patch<int, int> getPotato(0x6050607);

getPotato.before += [](int potato) {
  cout << "this will show before the original function is called" << endl;
};
getPotato.after += [](int potato) {
  cout << "original function already returned" << endl;
};
```

You can also replace the original function:
```cpp
// function |int getPotato(int arg0)| at address 0x6050607 
memory::Patch<int, int> getPotato(0x6050607);

getPotato.replace += [](int potato) {
  cout << "this will 'replace' all calls to the original function" << endl;
  return 60;
};
```

Some other functions that will help you read/write/execute from an address in memory:
```cpp
// calling function |int getPotato(int arg0)| at address 0x6050607 
memory::Call<int, 0x6050607, int>(40);
// reading int value at address 0x6050607
memory::Read<int>(0x6050607);
// writing int value at address 0x6050607
memory::Write<int>(0x6050607, 60);
```

# Contributions

Feel free to leave your contribution here, I would really appreciate it!
Also, if you have any doubts or troubles using this tool just contact me or leave an issue.
