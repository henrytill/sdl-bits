#include <stdio.h>

#include "prelude.h"

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
struct Person *Person_new(char *name, int age) {
  struct Person *self = emalloc(sizeof(struct Person));
  self->ops = &Person_ops;
  self->name = name;
  self->age = age;
  return self;
}

/// Base class destructor.
static void Person_destroy(struct Person *self) {
  free(self);
}

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
struct Student *Student_new(char *name, int age, char *school) {
  struct Student *self = emalloc(sizeof(struct Student));
  self->person.ops = &Student_ops;
  self->person.name = name;
  self->person.age = age;
  self->school = school;
  return self;
}

/// Derived class destructor.
static void Student_destroy(struct Student *self) {
  free(self);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(struct Person *, Person_destroy);
DEFINE_TRIVIAL_CLEANUP_FUNC(struct Student *, Student_destroy);
#define _cleanup_Person_  _cleanup_(Person_destroyp)
#define _cleanup_Student_ _cleanup_(Student_destroyp)

int main(_unused_ int argc, _unused_ char *argv[]) {
  _cleanup_Person_ struct Person *alice = Person_new("Alice", 20);
  _cleanup_Person_ struct Person *bob = Person_new("Bob", 21);
  _cleanup_Student_ struct Student *carol = Student_new("Carol", 22, "MIT");

  struct Person *people[] = {alice, bob, &carol->person};

  for (size_t i = 0; i < ARRAY_SIZE(people); ++i) {
    SEND(people[i], ops->sayHello);
  }

  return EXIT_SUCCESS;
}
