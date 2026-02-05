//
// Created by laadim on 26.01.26.
//

#include "../include/INode.h"

#include <stdexcept>

#include "../helpers/IntParser.h"

INode INode::FromBytes(std::vector<char> bytes) {
    if (bytes.size() != INode::BYTES) {
        throw std::runtime_error("INode::FromBytes size mismatch");
    }

    uint32_t offset = 0;

    auto readU32 = [&](uint32_t& out) {
        out = IntParser::ReadUInt32(
            std::vector<char>(bytes.begin() + offset,
                              bytes.begin() + offset + sizeof(uint32_t))
        );
        offset += sizeof(uint32_t);
    };

    INode inode;

    readU32(inode._id);
    readU32(inode._links);
    readU32(inode._size);

    for (int i = 0; i < DIRECT_LINKS; ++i) {
        readU32(inode._direct[i]);
    }

    readU32(inode._indirect1);
    readU32(inode._indirect2);


    // isDir: last byte (offset should now be 80)
    if (offset != INode::BYTES - 1) {
        throw std::runtime_error("INode::FromBytes offset mismatch");
    }

    const auto dirByte = static_cast<unsigned char>(bytes[offset]);
    if (dirByte != 0 && dirByte != 1) {
        throw std::runtime_error("INode::FromBytes invalid isDir value");
    }
    inode._isDir = (dirByte == 1);

    return inode;
}

std::vector<char> INode::ToBytes() const {
    std::vector<char> bytes;
    bytes.reserve(INode::BYTES);

    auto writeU32 = [&](uint32_t value) {
        auto data = IntParser::WriteUInt32(value);
        bytes.insert(bytes.end(), data.begin(), data.end());
    };

    writeU32(_id);
    writeU32(_links);
    writeU32(_size);

    for (int i = 0; i < DIRECT_LINKS; ++i) {
        writeU32(_direct[i]);
    }

    writeU32(_indirect1);
    writeU32(_indirect2);

    // isDir: exactly 1 byte
    bytes.push_back(_isDir ? 1 : 0);

    // Final safety check
    if (bytes.size() != INode::BYTES) {
        throw std::runtime_error("INode::ToBytes size mismatch");
    }

    return bytes;
}

INode::INode(const uint32_t id, const bool isDir)
    : _id(id),
      _links(1),
      _isDir(isDir),
      _size(0),
      _indirect1(UNUSED_LINK),
      _indirect2(UNUSED_LINK) {

    _direct = std::array<uint32_t, INode::DIRECT_LINKS>();
    for (auto& link : _direct) {
        link = UNUSED_LINK;
    }
}

INode::INode()
    : _id(0),
      _links(0),
      _isDir(false),
      _size(0),
      _indirect1(UNUSED_LINK),
      _indirect2(UNUSED_LINK) {

    _direct = std::array<uint32_t, INode::DIRECT_LINKS>();
    for (auto& link : _direct) {
        link = UNUSED_LINK;
    }
}

INode::~INode() = default;

uint32_t INode::getId() const {
    return _id;
}

bool INode::isDir() const {
    return _isDir;
}

uint32_t INode::getLinks() const {
    return _links;
}

void INode::addLink() {
    _links++;
}

bool INode::removeLink() {
    if (_links == 0) {
        return false;
    }
    _links--;
    return true;

}

uint32_t INode::getSize() const {
    return _size;
}

void INode::addSize(uint32_t bytes) {
    _size += bytes;
}

void INode::removeSize(uint32_t bytes) {
    if (bytes > _size) {
        throw std::runtime_error("INode::removeSize mismatch");
    }
    _size -= bytes;
}

std::array<uint32_t, INode::DIRECT_LINKS> INode::getDirectLinks() const {
    return _direct;
}

void INode::addDirectLink(const uint32_t link) {
    for (int i = 0; i < DIRECT_LINKS; ++i) {
        if (_direct[i] == INode::UNUSED_LINK) {
            _direct[i] = link;
            return;
        }
    }
    throw std::runtime_error("INode::addDirectLink mismatch");
}

void INode::removeDirectLink(const uint32_t link) {
    for (int i = 0; i < DIRECT_LINKS; ++i) {
        if (_direct[i] == link) {
            _direct[i] = INode::UNUSED_LINK;
            return;
        }
    }
    throw std::runtime_error("INode::removeDirectLink mismatch");
}

uint32_t INode::getFirstLevelIndirectLink() const {
    return _indirect1;
}

void INode::addFirstLevelIndirectLink(const uint32_t link) {
    if (_indirect1 != UNUSED_LINK) {
        throw std::runtime_error("INode::addFirstLevelIndirectLink mismatch");
    }
    _indirect1 = link;
}

void INode::removeFirstLevelIndirectLink() {
    _indirect1 = UNUSED_LINK;
}

uint32_t INode::getSecondLevelIndirectLink() const {
    return _indirect2;
}

void INode::addSecondLevelIndirectLink(const uint32_t link) {
    if (_indirect2 != UNUSED_LINK) {
        throw std::runtime_error("INode::addSecondLevelIndirectLink mismatch");
    }
    _indirect2 = link;
}

void INode::removeSecondLevelIndirectLink() {
    _indirect2 = UNUSED_LINK;
}

void INode::clearDirectLinks() {
    for (auto& link : _direct) {
        link = UNUSED_LINK;
    }
}
