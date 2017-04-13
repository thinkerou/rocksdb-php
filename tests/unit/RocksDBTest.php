<?php

class RocksDBTest extends PHPUnit_Framework_TestCase
{
    public function setUp()
    {
    }

    public function tearDown()
    {
    }

    public function testConstructor()
    {
        $options = array('create_if_missing' => true,);
        $db = new RocksDB\RocksDB('testdata', $options);
        $this->assertNotNull($db);
        $this->assertSame('RocksDB\RocksDB', get_class($db));
    
        $db->put("key_test", "value_test");
        $this->assertEquals($db->get("key_test"), "value_test");

        // TODO: get("aaa", "bbb") have two params that will crash
        // TODO: get("no_exists_key") should return?
        // $this->assertEquals($db->get("null_key"), "null_value");
    }

}
