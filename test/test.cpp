#include "test.h"

int main()
{
  try {
    _InitCli();
    //ConfigTest();
    ProcessTest();
  }
  catch (const exception& e) {
    cout << e.what() << endl << flush;
    _TerminateCli(EXIT_FAILURE);
  }
  _TerminateCli(EXIT_SUCCESS);
}

void _InitCli()
{
  _set_error_mode(_OUT_TO_STDERR);
  cout << _format("36", "\n\tRunning tests\n\n") << flush;
}

void _TerminateCli(int code)
{
  if (!code)
    cout << _format("32", "\n\tSuccessfully finished tests\n\n") << flush;
  exit(code);
}
