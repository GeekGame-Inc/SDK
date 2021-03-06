#include <stdbool.h>
#include "c/ModioC.h"
#include <stdio.h>

void onAddModTags(void *object, ModioResponse response)
{
  bool *wait = object;
  printf("Add tags response: %i\n", response.code);
  if (response.code == 201)
  {
    printf("Tags added successfully!\n");
  }
  *wait = false;
}

int main(void)
{
  modioInit(MODIO_ENVIRONMENT_TEST, 7, false, "e91c01b8882f4affeddd56c96111977b", NULL);

  bool wait = true;

  // Let's start by requesting a single mod
  printf("Please enter the mod id: \n");
  u32 mod_id;
  scanf("%i", &mod_id);

  printf("Adding tags...\n");

  char const *tags_array[1];
  tags_array[0] = "Hard";

  // We add tags to a mod by providing the tag names. Remember, they must be valid tags allowed by the parrent game
  modioAddModTags(&wait, mod_id, tags_array, 1, &onAddModTags);

  while (wait)
  {
    modioProcess();
  }

  modioShutdown();

  printf("Process finished\n");

  return 0;
}
