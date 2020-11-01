代码来自于io.prestosql.tpch

```java

class Demo {

    void generateDataFile() {
        try(Writer writer = new FileWriter("yourFile")){
            for (Customer entity : new CustomerGenerator(scaleFactor, part, numberOfParts)) {
                writer.write(entity.toLine());
                writer.write('\n');
            }
        }
    }

}

```
