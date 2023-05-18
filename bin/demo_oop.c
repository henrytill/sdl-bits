#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/// Base class methods.
typedef struct PersonOperations PersonOperations;

/// Base class.
typedef struct Person Person;

struct PersonOperations {
  void (*SayHello)(const Person* self);
};

struct Person {
  const PersonOperations* ops;
  char* name;
  int age;
};

static void Person_SayHello(const Person* self) {
  printf("Hello, my name is %s, I'm %d years old.\n", self->name, self->age);
}

/// Base class vtable.
static const PersonOperations Person_ops = {
  .SayHello = Person_SayHello,
};

/// Base class constructor.
#define Person(_name, _age) \
  ((Person){                \
    .ops = &Person_ops,     \
    .name = (_name),        \
    .age = (_age),          \
  })

/// Derived class.
typedef struct Student {
  Person person;
  char* school;
} Student;

/// Derived class override of PersonOperations::SayHello
static void Student_SayHello(const Person* self) {
  const Student* student = CONTAINER_OF(self, Student, person);
  printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
         student->person.name, student->person.age, student->school);
}

/// Derived class vtable.
static const PersonOperations Student_ops = {
  .SayHello = Student_SayHello,
};

/// Derived class constructor.
#define Student(_name, _age, _school) \
  ((Student){                         \
    .person = {                       \
      .ops = &Student_ops,            \
      .name = (_name),                \
      .age = (_age),                  \
    },                                \
    .school = (_school),              \
  })

int main(_unused_ int argc, _unused_ char* argv[]) {
  Person alice = Person("Alice", 20);
  Person bob = Person("Bob", 21);
  Student carol = Student("Carol", 22, "MIT");

  Person* people[] = {&alice, &bob, &carol.person};

  for (size_t i = 0; i < ARRAY_SIZE(people); ++i)
    SEND(people[i], ops->SayHello);

  return EXIT_SUCCESS;
}
