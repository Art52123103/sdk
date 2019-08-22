/**
 * (c) 2019 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include <atomic>
#include <memory>
#include <numeric>
#include <thread>

#include <gtest/gtest.h>

#include <mega.h>
#include <megaapi.h>

#include "utils.h"

TEST(Serialization, JSON_storeobject)
{
    std::string in_str("Test");
    mega::JSON j;
    j.begin(in_str.data());
    j.storeobject(&in_str);
}

// Test 64-bit int serialization/unserialization
TEST(Serialization, Serialize64_serialize)
{
    uint64_t in = 0xDEADBEEF;
    uint64_t out;
    mega::byte buf[sizeof in];

    mega::Serialize64::serialize(buf, in);
    ASSERT_GT(mega::Serialize64::unserialize(buf, sizeof buf, &out), 0);
    ASSERT_EQ(in, out);
}

TEST(Serialization, CacheableReaderWriter)
{
    auto checksize = [](size_t& n, size_t added)
    {
        n += added;
        return n;
    };

    std::string writestring;
    mega::CacheableWriter w(writestring);

    mega::byte binary[] = { 1, 2, 3, 4, 5 };
    std::string cstr1("test1");
    std::string cstr2("test2diffdata");
    std::string stringtest("diffstringagaindefinitelybigger");
    int64_t i64 = 0x8765432112345678;
    uint32_t u32 = 0x87678765;
    mega::handle handle1 = 0x998;
    bool b = true;
    mega::byte by = 5;
    mega::chunkmac_map cm;
    cm[777].offset = 888;

    size_t sizeadded = 0;

    w.serializebinary(binary, sizeof(binary));
    ASSERT_EQ(writestring.size(), checksize(sizeadded, sizeof(binary)));

    w.serializecstr(cstr1.c_str(), true);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 2 + cstr1.size() + 1));

    w.serializecstr(cstr2.c_str(), false);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 2 + cstr2.size()));

    w.serializestring(stringtest);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 2 + stringtest.size()));

    w.serializei64(i64);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 8));

    w.serializeu32(u32);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 4));

    w.serializehandle(handle1);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, sizeof(mega::handle)));

    w.serializebool(b);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, sizeof(bool)));

    w.serializebyte(by);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 1));

    w.serializechunkmacs(cm);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 2 + 1 * (sizeof(m_off_t) + sizeof(mega::ChunkMAC))));

    w.serializeexpansionflags(1, 0, 1, 0, 0, 0, 1, 1);
    ASSERT_EQ(writestring.size(), checksize(sizeadded, 8));

    writestring += "abc";

    // now read the serialized data back
    std::string readstring = writestring;
    mega::CacheableReader r(readstring);

    mega::byte check_binary[5];
    std::string check_cstr1;
    std::string check_cstr2;
    std::string check_stringtest;
    int64_t check_i64;
    uint32_t check_u32;
    mega::handle check_handle1;
    bool check_b;
    mega::byte check_by;
    mega::chunkmac_map check_cm;

    ASSERT_TRUE(r.unserializebinary(check_binary, sizeof(check_binary)));
    ASSERT_EQ(0, memcmp(check_binary, binary, sizeof(binary)));

    ASSERT_TRUE(r.unserializecstr(check_cstr1, true));
    ASSERT_EQ(check_cstr1, cstr1);

    ASSERT_TRUE(r.unserializecstr(check_cstr2, false));
    ASSERT_EQ(check_cstr2, cstr2);

    ASSERT_TRUE(r.unserializestring(check_stringtest));
    ASSERT_EQ(check_stringtest, stringtest);

    ASSERT_TRUE(r.unserializei64(check_i64));
    ASSERT_EQ(check_i64, i64);

    ASSERT_TRUE(r.unserializeu32(check_u32));
    ASSERT_EQ(check_u32, u32);

    ASSERT_TRUE(r.unserializehandle(check_handle1));
    ASSERT_EQ(check_handle1, handle1);

    ASSERT_TRUE(r.unserializebool(check_b));
    ASSERT_EQ(check_b, b);

    ASSERT_TRUE(r.unserializebyte(check_by));
    ASSERT_EQ(check_by, by);

    ASSERT_TRUE(r.unserializechunkmacs(check_cm));
    ASSERT_EQ(check_cm[777].offset, cm[777].offset);

    unsigned char expansions[8];
    ASSERT_FALSE(r.unserializeexpansionflags(expansions, 7));
    ASSERT_TRUE(r.unserializeexpansionflags(expansions, 8));
    ASSERT_EQ(expansions[0], 1);
    ASSERT_EQ(expansions[1], 0);
    ASSERT_EQ(expansions[2], 1);
    ASSERT_EQ(expansions[3], 0);
    ASSERT_EQ(expansions[4], 0);
    ASSERT_EQ(expansions[5], 0);
    ASSERT_EQ(expansions[6], 1);
    ASSERT_EQ(expansions[7], 1);

    r.eraseused(readstring);
    ASSERT_EQ(readstring, "abc");

    mega::MediaProperties mp;
    mp.shortformat = 1;
    mp.width = 2;
    mp.height = 3;
    mp.fps = 4;
    mp.playtime = 5;
    mp.containerid = 6;
    mp.videocodecid = 7;
    mp.audiocodecid = 8;
    mp.is_VFR = true;
    mp.no_audio = false;
    std::string mps = mp.serialize();
    mega::MediaProperties mp2(mps);
    ASSERT_EQ(mps, mp2.serialize());
    ASSERT_EQ(mp2.shortformat, 1);
    ASSERT_EQ(mp2.width, 2u);
    ASSERT_EQ(mp2.height, 3u);
    ASSERT_EQ(mp2.fps, 4u);
    ASSERT_EQ(mp2.playtime, 5u);
    ASSERT_EQ(mp2.containerid, 6u);
    ASSERT_EQ(mp2.videocodecid, 7u);
    ASSERT_EQ(mp2.audiocodecid, 8u);
    ASSERT_EQ(mp2.is_VFR, true);
    ASSERT_EQ(mp2.no_audio, false);
}

namespace {

void checkDeserializedLocalNode(const mega::LocalNode& dl, const mega::LocalNode& ref)
{
    ASSERT_EQ(ref.type, dl.type);
    ASSERT_EQ(ref.size < 0 ? 0 : ref.size, dl.size);
    ASSERT_EQ(ref.parent_dbid, dl.parent_dbid);
    ASSERT_EQ(ref.fsid, dl.fsid);
    ASSERT_EQ(ref.localname, dl.localname);
    ASSERT_EQ(nullptr, dl.slocalname);
    ASSERT_EQ(ref.name, dl.name);
    ASSERT_EQ(mt::toArr(ref.crc), mt::toArr(dl.crc));
    ASSERT_EQ(ref.mtime, dl.mtime);
    ASSERT_EQ(true, dl.isvalid);
    ASSERT_EQ(nullptr, dl.parent);
    ASSERT_EQ(ref.sync, dl.sync);
//    ASSERT_EQ(ref.syncable, dl.syncable);
    ASSERT_EQ(false, dl.created);
    ASSERT_EQ(false, dl.reported);
    ASSERT_EQ(true, dl.checked);
}

}

TEST(Serialization, LocalNode_shortData)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    std::string data = "I am too short";
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    ASSERT_EQ(nullptr, dl);
}

TEST(Serialization, LocalNode_forFolder_withoutParent_withoutNode)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    auto& l = sync->localroot;
    l.syncable = false;
    l.setfsid(10, fsidnodes);
    std::string data;
    ASSERT_TRUE(l.serialize(&data));
    ASSERT_EQ(34, data.size());
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    checkDeserializedLocalNode(*dl, l);
}

TEST(Serialization, LocalNode_forFile_withoutNode)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    auto l = mt::makeLocalNode(*sync, sync->localroot, fsidnodes, mega::FILENODE, "sweet");
    l->syncable = false;
    l->size = 124;
    l->setfsid(10, fsidnodes);
    l->parent->dbid = 13;
    l->parent_dbid = l->parent->dbid;
    l->mtime = 124124124;
    std::iota(l->crc, l->crc + mega::getSize(l->crc), 1);
    std::string data;
    ASSERT_TRUE(l->serialize(&data));
    ASSERT_EQ(54, data.size());
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    checkDeserializedLocalNode(*dl, *l);
}

TEST(Serialization, LocalNode_forFile_withoutNode_withMaxMtime)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    auto l = mt::makeLocalNode(*sync, sync->localroot, fsidnodes, mega::FILENODE, "sweet");
    l->size = 124;
    l->setfsid(10, fsidnodes);
    l->parent->dbid = 13;
    l->parent_dbid = l->parent->dbid;
    l->mtime = std::numeric_limits<decltype(l->mtime)>::max();
    std::iota(l->crc, l->crc + mega::getSize(l->crc), 1);
    std::string data;
    ASSERT_TRUE(l->serialize(&data));
    ASSERT_EQ(58, data.size());
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    checkDeserializedLocalNode(*dl, *l);
}

TEST(Serialization, LocalNode_forFolder_withoutParent)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    mega::Node n;
    n.nodehandle = 42;
    auto& l = sync->localroot;
    l.setfsid(10, fsidnodes);
    l.node = &n;
    std::string data;
    ASSERT_TRUE(l.serialize(&data));
    ASSERT_EQ(34, data.size());
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    checkDeserializedLocalNode(*dl, l);
}

TEST(Serialization, LocalNode_forFolder)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    auto l = mt::makeLocalNode(*sync, sync->localroot, fsidnodes, mega::FOLDERNODE, "sweet");
    l->syncable = false;
    l->parent->dbid = 13;
    l->parent_dbid = l->parent->dbid;
    mega::Node n;
    n.nodehandle = 42;
    l->setfsid(10, fsidnodes);
    l->node = &n;
    std::string data;
    ASSERT_TRUE(l->serialize(&data));
    ASSERT_EQ(33, data.size());
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    checkDeserializedLocalNode(*dl, *l);
}

TEST(Serialization, LocalNode_forFile)
{
    mega::handlelocalnode_map fsidnodes;
    auto sync = mt::makeSync("wicked", fsidnodes);
    auto l = mt::makeLocalNode(*sync, sync->localroot, fsidnodes, mega::FILENODE, "sweet");
    l->syncable = false;
    mega::Node n;
    n.nodehandle = 42;
    l->node = &n;
    l->size = 1;
    l->setfsid(10, fsidnodes);
    l->parent->dbid = 13;
    l->parent_dbid = l->parent->dbid;
    l->mtime = 0;
    std::iota(l->crc, l->crc + mega::getSize(l->crc), 1);
    std::string data;
    ASSERT_TRUE(l->serialize(&data));
    ASSERT_EQ(50, data.size());
    auto dl = std::unique_ptr<mega::LocalNode>{mega::LocalNode::unserialize(sync.get(), &data)};
    checkDeserializedLocalNode(*dl, *l);
}
