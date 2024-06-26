#include "gtest/gtest.h"
#include "ctemplate.hpp"
#include "helper.hpp"
#include "os.hpp"

namespace path = os::path;

std::string test_path = path::joinPath(path::sourcePath(), "../test_path");
std::string template_path = path::joinPath(test_path, "templates");
std::string temp_path = path::joinPath(test_path, "temp");

TEST(addTemplate, adding)
{
    std::string add_path = path::joinPath(template_path, "t1");
    std::string name = "testing";
    std::string desc = "wassup boi";
    std::string container_name = ".ctemplate";
    addTemplate(template_path, add_path, name, desc, container_name);

    std::string new_template = path::joinPath(template_path, "testing");
    ASSERT_TRUE(path::exists(new_template));
    ASSERT_TRUE(path::exists(path::joinPath(new_template, container_name)));
    
    path::remove(new_template);
}

TEST(addTemplate, empty_name)
{
    std::string add_path = path::joinPath(template_path, "t1");
    std::string name = "testing";
    std::string desc = "wassup boi";
    std::string container_name = ".ctemplate";
    addTemplate(template_path, add_path, "", desc, container_name);

    ASSERT_TRUE(!path::exists(name));
}

TEST(addTemplate, existing_container)
{
    std::string add_path = path::joinPath(template_path, "t1");
    std::string name = "testing";
    std::string desc = "wassup boi";
    std::string container_name = ".ctemplate";
    addTemplate(template_path, add_path, name, desc, container_name);

    std::string new_template = path::joinPath(template_path, "testing");
    std::string new_container = path::joinPath(new_template, ".ctemplate");
    ASSERT_TRUE(path::exists(path::joinPath(new_container, "info.json")));
    ASSERT_TRUE(path::exists(path::joinPath(new_container, "variables.json")));

    path::remove(new_template);
}

TEST(removeTemplates, removing)
{
    path::copy(path::joinPath(template_path, "t1"), temp_path);
    path::copy(path::joinPath(template_path, "t2"), temp_path);

    ASSERT_TRUE(path::exists(path::joinPath(template_path, "t1")));
    ASSERT_TRUE(path::exists(path::joinPath(template_path, "t2")));
    removeTemplates(template_path, {"t1", "t2"});
    ASSERT_TRUE(!path::exists(path::joinPath(template_path, "t1")));
    ASSERT_TRUE(!path::exists(path::joinPath(template_path, "t2")));

    path::copy(path::joinPath(temp_path, "t1"), template_path, path::CopyOption::OverwriteExisting);
    path::copy(path::joinPath(temp_path, "t2"), template_path, path::CopyOption::OverwriteExisting);

    path::remove(path::joinPath(temp_path, "t1"));
    path::remove(path::joinPath(temp_path, "t2"));
}

TEST(split, splitting)
{
    std::vector<std::string> s = split("key=value", "=");
    EXPECT_EQ(s, std::vector<std::string>({"key", "value"}));
}

TEST(mapKeyValues, mapping)
{
    std::vector<std::string> s = {"project=hello", "name=johnny sins", "order=asc", "something"};
    std::unordered_map<std::string, std::string> m = mapKeyValues(s);
    std::unordered_map<std::string, std::string> expected_m = {{"project", "hello"}, {"name", "johnny sins"}, {"order", "asc"}};
    EXPECT_EQ(m, expected_m);
}

TEST(replaceVariables, replacing)
{
    std::string str = "My name is !name! and I am !age! years old.";
    std::string prefix = "!";
    std::string suffix = "!";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is John and I am 12 years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, longer_prefix_and_suffix)
{
    std::string str = "My name is {!name!} and I am {!age!} years old.";
    std::string prefix = "{!";
    std::string suffix = "!}";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is John and I am 12 years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, uneven_prefix_and_suffix)
{
    std::string str = "My name is {!name!}] and I am {!age!}] years old.";
    std::string prefix = "{!";
    std::string suffix = "!}]";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is John and I am 12 years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, missing_prefix)
{
    std::string str = "My name is name!}] and I am age!}] years old.";
    std::string prefix = "{!";
    std::string suffix = "!}]";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is name!}] and I am age!}] years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, missing_suffix)
{
    std::string str = "My name is {!name and I am {!age years old.";
    std::string prefix = "{!";
    std::string suffix = "!}]";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is {!name and I am {!age years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, missing_variables)
{
    std::string str = "My name is !name! and";
    std::string prefix = "!";
    std::string suffix = "!";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is John and";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, endlines_in_str)
{
    std::string str = "My name is {!name!}\nand I am\n{!age!} years old.";
    std::string prefix = "{!";
    std::string suffix = "!}";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is John\nand I am\n12 years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, endlines_in_var)
{
    std::string str = "My name is {!name!} and I am {!age!} years old.";
    std::string prefix = "{!";
    std::string suffix = "!}";
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John\nWick"}, {"age", "12\nteen"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is John\nWick and I am 12\nteen years old.";
    EXPECT_EQ(actual, expected);
}

TEST(replaceVariables, edge_cases)
{
    std::string str = "My name is !name! and I am !age! years old.";
    std::string prefix = ""; // no prefix
    std::string suffix = ""; // no suffix
    std::unordered_map<std::string, std::string> keyvals = {{"name", "John"}, {"age", "12"}};

    std::string actual = replaceVariables(str, keyvals, prefix, suffix);
    std::string expected = "My name is !name! and I am !age! years old.";
    EXPECT_EQ(actual, expected);
}

TEST(compileIncludedPaths, include_all_with_excludes)
{
    std::unordered_set<std::string> actual = compileIncludedPaths(path::joinPath(template_path, "cpp-test"),
            std::unordered_set<std::string>(), std::unordered_set<std::string>({"src/main.cpp"}));
    std::unordered_set<std::string> expected = {"src\\temp.cpp", "CMakeLists.txt", ".gitignore", ".ctemplate\\info.json"};

    EXPECT_EQ(actual, expected);

    actual = compileIncludedPaths(path::joinPath(template_path, "cpp-test"),
            std::unordered_set<std::string>(), std::unordered_set<std::string>({"src/main.cpp", ".ctemplate"}));
    expected = {"src\\temp.cpp", "CMakeLists.txt", ".gitignore"};

    EXPECT_EQ(actual, expected);
}