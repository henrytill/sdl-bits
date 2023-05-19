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
  Person* next;
};

static void Person_SayHello(const Person* self) {
  printf("Hello, my name is %s, I'm %d years old.\n", self->name, self->age);
}

/// Base class vtable.
static const PersonOperations Person_ops = {
  .SayHello = Person_SayHello,
};

/// Base class constructor.
#define Person(_name, _age, _next) \
  ((Person){                       \
    .ops = &Person_ops,            \
    .name = (_name),               \
    .age = (_age),                 \
    .next = (_next),               \
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
#define Student(_name, _age, _next, _school) \
  ((Student){                                \
    .person = {                              \
      .ops = &Student_ops,                   \
      .name = (_name),                       \
      .age = (_age),                         \
      .next = (_next),                       \
    },                                       \
    .school = (_school),                     \
  })

int main(void) {
  Person alice = Person("Alice", 20, NULL);
  Person bob = Person("Bob", 21, &alice);
  Student carol = Student("Carol", 22, &bob, "MIT");

  for (const Person* current = &carol.person; current != NULL; current = current->next) {
    SEND(current, ops->SayHello);
  }

  return EXIT_SUCCESS;
}
