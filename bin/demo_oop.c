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

int main(void) {
  Person alice;
  Person bob;
  Student carol;

  alice = (Person){
    .ops = &Person_ops,
    .name = "Alice",
    .age = 20,
    .next = &bob,
  };
  bob = (Person){
    .ops = &Person_ops,
    .name = "Bob",
    .age = 21,
    .next = &carol.person,
  };
  carol = (Student){
    .person = {
      .ops = &Student_ops,
      .name = "Carol",
      .age = 22,
      .next = NULL,
    },
    .school = "MIT",
  };

  for (const Person* p = &alice; p != NULL; p = p->next) {
    SEND(p, ops->SayHello);
  }

  return EXIT_SUCCESS;
}
