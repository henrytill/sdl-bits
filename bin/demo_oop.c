#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/// Base class methods.
struct PersonOperations;

/// Base class.
struct Person {
  const struct PersonOperations *ops;
  char *name;
  int age;
};

struct PersonOperations {
  void (*sayHello)(struct Person *self);
};

static void Person_sayHello(struct Person *self) {
  printf("Hello, my name is %s, I'm %d years old.\n", self->name, self->age);
}

/// Base class vtable.
static const struct PersonOperations Person_ops = {
  .sayHello = Person_sayHello,
};

/// Base class constructor.
#define Person(__name, __age) \
  ((struct Person){           \
    .ops = &Person_ops,       \
    .name = (__name),         \
    .age = (__age),           \
  })

/// Derived class.
struct Student {
  struct Person person;
  char *school;
};

/// Derived class override of PersonOperations::sayHello
static void Student_sayHello(struct Person *self) {
  struct Student *student = CONTAINER_OF(self, struct Student, person);
  printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
         student->person.name, student->person.age, student->school);
}

/// Derived class vtable.
static const struct PersonOperations Student_ops = {
  .sayHello = Student_sayHello,
};

/// Derived class constructor.
#define Student(__name, __age, __school) \
  ((struct Student){                     \
    .person = {                          \
      .ops = &Student_ops,               \
      .name = (__name),                  \
      .age = (__age),                    \
    },                                   \
    .school = (__school),                \
  })

int main(_unused_ int argc, _unused_ char *argv[]) {
  struct Person alice = Person("Alice", 20);
  struct Person bob = Person("Bob", 21);
  struct Student carol = Student("Carol", 22, "MIT");

  struct Person *people[] = {&alice, &bob, &carol.person};

  for (size_t i = 0; i < ARRAY_SIZE(people); ++i) {
    SEND(people[i], ops->sayHello);
  }

  return EXIT_SUCCESS;
}
