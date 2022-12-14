// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: db.proto

#ifndef PROTOBUF_db_2eproto__INCLUDED
#define PROTOBUF_db_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/service.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace db {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_db_2eproto();
void protobuf_AssignDesc_db_2eproto();
void protobuf_ShutdownFile_db_2eproto();

class ClientRequest;
class ClientResponse;
class ClusterRequest;
class ClusterResponse;

// ===================================================================

class ClientRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:db.ClientRequest) */ {
 public:
  ClientRequest();
  virtual ~ClientRequest();

  ClientRequest(const ClientRequest& from);

  inline ClientRequest& operator=(const ClientRequest& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ClientRequest& default_instance();

  void Swap(ClientRequest* other);

  // implements Message ----------------------------------------------

  inline ClientRequest* New() const { return New(NULL); }

  ClientRequest* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ClientRequest& from);
  void MergeFrom(const ClientRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ClientRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required string msg_type = 1;
  bool has_msg_type() const;
  void clear_msg_type();
  static const int kMsgTypeFieldNumber = 1;
  const ::std::string& msg_type() const;
  void set_msg_type(const ::std::string& value);
  void set_msg_type(const char* value);
  void set_msg_type(const char* value, size_t size);
  ::std::string* mutable_msg_type();
  ::std::string* release_msg_type();
  void set_allocated_msg_type(::std::string* msg_type);

  // required string msg = 2;
  bool has_msg() const;
  void clear_msg();
  static const int kMsgFieldNumber = 2;
  const ::std::string& msg() const;
  void set_msg(const ::std::string& value);
  void set_msg(const char* value);
  void set_msg(const char* value, size_t size);
  ::std::string* mutable_msg();
  ::std::string* release_msg();
  void set_allocated_msg(::std::string* msg);

  // @@protoc_insertion_point(class_scope:db.ClientRequest)
 private:
  inline void set_has_msg_type();
  inline void clear_has_msg_type();
  inline void set_has_msg();
  inline void clear_has_msg();

  // helper for ByteSize()
  int RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr msg_type_;
  ::google::protobuf::internal::ArenaStringPtr msg_;
  friend void  protobuf_AddDesc_db_2eproto();
  friend void protobuf_AssignDesc_db_2eproto();
  friend void protobuf_ShutdownFile_db_2eproto();

  void InitAsDefaultInstance();
  static ClientRequest* default_instance_;
};
// -------------------------------------------------------------------

class ClientResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:db.ClientResponse) */ {
 public:
  ClientResponse();
  virtual ~ClientResponse();

  ClientResponse(const ClientResponse& from);

  inline ClientResponse& operator=(const ClientResponse& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ClientResponse& default_instance();

  void Swap(ClientResponse* other);

  // implements Message ----------------------------------------------

  inline ClientResponse* New() const { return New(NULL); }

  ClientResponse* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ClientResponse& from);
  void MergeFrom(const ClientResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ClientResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required string msg = 1;
  bool has_msg() const;
  void clear_msg();
  static const int kMsgFieldNumber = 1;
  const ::std::string& msg() const;
  void set_msg(const ::std::string& value);
  void set_msg(const char* value);
  void set_msg(const char* value, size_t size);
  ::std::string* mutable_msg();
  ::std::string* release_msg();
  void set_allocated_msg(::std::string* msg);

  // @@protoc_insertion_point(class_scope:db.ClientResponse)
 private:
  inline void set_has_msg();
  inline void clear_has_msg();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr msg_;
  friend void  protobuf_AddDesc_db_2eproto();
  friend void protobuf_AssignDesc_db_2eproto();
  friend void protobuf_ShutdownFile_db_2eproto();

  void InitAsDefaultInstance();
  static ClientResponse* default_instance_;
};
// -------------------------------------------------------------------

class ClusterRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:db.ClusterRequest) */ {
 public:
  ClusterRequest();
  virtual ~ClusterRequest();

  ClusterRequest(const ClusterRequest& from);

  inline ClusterRequest& operator=(const ClusterRequest& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ClusterRequest& default_instance();

  void Swap(ClusterRequest* other);

  // implements Message ----------------------------------------------

  inline ClusterRequest* New() const { return New(NULL); }

  ClusterRequest* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ClusterRequest& from);
  void MergeFrom(const ClusterRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ClusterRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required string msg_type = 1;
  bool has_msg_type() const;
  void clear_msg_type();
  static const int kMsgTypeFieldNumber = 1;
  const ::std::string& msg_type() const;
  void set_msg_type(const ::std::string& value);
  void set_msg_type(const char* value);
  void set_msg_type(const char* value, size_t size);
  ::std::string* mutable_msg_type();
  ::std::string* release_msg_type();
  void set_allocated_msg_type(::std::string* msg_type);

  // required string msg = 2;
  bool has_msg() const;
  void clear_msg();
  static const int kMsgFieldNumber = 2;
  const ::std::string& msg() const;
  void set_msg(const ::std::string& value);
  void set_msg(const char* value);
  void set_msg(const char* value, size_t size);
  ::std::string* mutable_msg();
  ::std::string* release_msg();
  void set_allocated_msg(::std::string* msg);

  // @@protoc_insertion_point(class_scope:db.ClusterRequest)
 private:
  inline void set_has_msg_type();
  inline void clear_has_msg_type();
  inline void set_has_msg();
  inline void clear_has_msg();

  // helper for ByteSize()
  int RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr msg_type_;
  ::google::protobuf::internal::ArenaStringPtr msg_;
  friend void  protobuf_AddDesc_db_2eproto();
  friend void protobuf_AssignDesc_db_2eproto();
  friend void protobuf_ShutdownFile_db_2eproto();

  void InitAsDefaultInstance();
  static ClusterRequest* default_instance_;
};
// -------------------------------------------------------------------

class ClusterResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:db.ClusterResponse) */ {
 public:
  ClusterResponse();
  virtual ~ClusterResponse();

  ClusterResponse(const ClusterResponse& from);

  inline ClusterResponse& operator=(const ClusterResponse& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ClusterResponse& default_instance();

  void Swap(ClusterResponse* other);

  // implements Message ----------------------------------------------

  inline ClusterResponse* New() const { return New(NULL); }

  ClusterResponse* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ClusterResponse& from);
  void MergeFrom(const ClusterResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ClusterResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required string msg = 1;
  bool has_msg() const;
  void clear_msg();
  static const int kMsgFieldNumber = 1;
  const ::std::string& msg() const;
  void set_msg(const ::std::string& value);
  void set_msg(const char* value);
  void set_msg(const char* value, size_t size);
  ::std::string* mutable_msg();
  ::std::string* release_msg();
  void set_allocated_msg(::std::string* msg);

  // @@protoc_insertion_point(class_scope:db.ClusterResponse)
 private:
  inline void set_has_msg();
  inline void clear_has_msg();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr msg_;
  friend void  protobuf_AddDesc_db_2eproto();
  friend void protobuf_AssignDesc_db_2eproto();
  friend void protobuf_ShutdownFile_db_2eproto();

  void InitAsDefaultInstance();
  static ClusterResponse* default_instance_;
};
// ===================================================================

class ClusterService_Stub;

class ClusterService : public ::google::protobuf::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline ClusterService() {};
 public:
  virtual ~ClusterService();

  typedef ClusterService_Stub Stub;

  static const ::google::protobuf::ServiceDescriptor* descriptor();

  virtual void SendClientMsg(::google::protobuf::RpcController* controller,
                       const ::db::ClientRequest* request,
                       ::db::ClientResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::google::protobuf::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  ::google::protobuf::RpcController* controller,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done);
  const ::google::protobuf::Message& GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const;
  const ::google::protobuf::Message& GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ClusterService);
};

class ClusterService_Stub : public ClusterService {
 public:
  ClusterService_Stub(::google::protobuf::RpcChannel* channel);
  ClusterService_Stub(::google::protobuf::RpcChannel* channel,
                   ::google::protobuf::Service::ChannelOwnership ownership);
  ~ClusterService_Stub();

  inline ::google::protobuf::RpcChannel* channel() { return channel_; }

  // implements ClusterService ------------------------------------------

  void SendClientMsg(::google::protobuf::RpcController* controller,
                       const ::db::ClientRequest* request,
                       ::db::ClientResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::google::protobuf::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ClusterService_Stub);
};


// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ClientRequest

// required string msg_type = 1;
inline bool ClientRequest::has_msg_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ClientRequest::set_has_msg_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ClientRequest::clear_has_msg_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ClientRequest::clear_msg_type() {
  msg_type_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_msg_type();
}
inline const ::std::string& ClientRequest::msg_type() const {
  // @@protoc_insertion_point(field_get:db.ClientRequest.msg_type)
  return msg_type_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClientRequest::set_msg_type(const ::std::string& value) {
  set_has_msg_type();
  msg_type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:db.ClientRequest.msg_type)
}
inline void ClientRequest::set_msg_type(const char* value) {
  set_has_msg_type();
  msg_type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:db.ClientRequest.msg_type)
}
inline void ClientRequest::set_msg_type(const char* value, size_t size) {
  set_has_msg_type();
  msg_type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:db.ClientRequest.msg_type)
}
inline ::std::string* ClientRequest::mutable_msg_type() {
  set_has_msg_type();
  // @@protoc_insertion_point(field_mutable:db.ClientRequest.msg_type)
  return msg_type_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ClientRequest::release_msg_type() {
  // @@protoc_insertion_point(field_release:db.ClientRequest.msg_type)
  clear_has_msg_type();
  return msg_type_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClientRequest::set_allocated_msg_type(::std::string* msg_type) {
  if (msg_type != NULL) {
    set_has_msg_type();
  } else {
    clear_has_msg_type();
  }
  msg_type_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg_type);
  // @@protoc_insertion_point(field_set_allocated:db.ClientRequest.msg_type)
}

// required string msg = 2;
inline bool ClientRequest::has_msg() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ClientRequest::set_has_msg() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ClientRequest::clear_has_msg() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ClientRequest::clear_msg() {
  msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_msg();
}
inline const ::std::string& ClientRequest::msg() const {
  // @@protoc_insertion_point(field_get:db.ClientRequest.msg)
  return msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClientRequest::set_msg(const ::std::string& value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:db.ClientRequest.msg)
}
inline void ClientRequest::set_msg(const char* value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:db.ClientRequest.msg)
}
inline void ClientRequest::set_msg(const char* value, size_t size) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:db.ClientRequest.msg)
}
inline ::std::string* ClientRequest::mutable_msg() {
  set_has_msg();
  // @@protoc_insertion_point(field_mutable:db.ClientRequest.msg)
  return msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ClientRequest::release_msg() {
  // @@protoc_insertion_point(field_release:db.ClientRequest.msg)
  clear_has_msg();
  return msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClientRequest::set_allocated_msg(::std::string* msg) {
  if (msg != NULL) {
    set_has_msg();
  } else {
    clear_has_msg();
  }
  msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg);
  // @@protoc_insertion_point(field_set_allocated:db.ClientRequest.msg)
}

// -------------------------------------------------------------------

// ClientResponse

// required string msg = 1;
inline bool ClientResponse::has_msg() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ClientResponse::set_has_msg() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ClientResponse::clear_has_msg() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ClientResponse::clear_msg() {
  msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_msg();
}
inline const ::std::string& ClientResponse::msg() const {
  // @@protoc_insertion_point(field_get:db.ClientResponse.msg)
  return msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClientResponse::set_msg(const ::std::string& value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:db.ClientResponse.msg)
}
inline void ClientResponse::set_msg(const char* value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:db.ClientResponse.msg)
}
inline void ClientResponse::set_msg(const char* value, size_t size) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:db.ClientResponse.msg)
}
inline ::std::string* ClientResponse::mutable_msg() {
  set_has_msg();
  // @@protoc_insertion_point(field_mutable:db.ClientResponse.msg)
  return msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ClientResponse::release_msg() {
  // @@protoc_insertion_point(field_release:db.ClientResponse.msg)
  clear_has_msg();
  return msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClientResponse::set_allocated_msg(::std::string* msg) {
  if (msg != NULL) {
    set_has_msg();
  } else {
    clear_has_msg();
  }
  msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg);
  // @@protoc_insertion_point(field_set_allocated:db.ClientResponse.msg)
}

// -------------------------------------------------------------------

// ClusterRequest

// required string msg_type = 1;
inline bool ClusterRequest::has_msg_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ClusterRequest::set_has_msg_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ClusterRequest::clear_has_msg_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ClusterRequest::clear_msg_type() {
  msg_type_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_msg_type();
}
inline const ::std::string& ClusterRequest::msg_type() const {
  // @@protoc_insertion_point(field_get:db.ClusterRequest.msg_type)
  return msg_type_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClusterRequest::set_msg_type(const ::std::string& value) {
  set_has_msg_type();
  msg_type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:db.ClusterRequest.msg_type)
}
inline void ClusterRequest::set_msg_type(const char* value) {
  set_has_msg_type();
  msg_type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:db.ClusterRequest.msg_type)
}
inline void ClusterRequest::set_msg_type(const char* value, size_t size) {
  set_has_msg_type();
  msg_type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:db.ClusterRequest.msg_type)
}
inline ::std::string* ClusterRequest::mutable_msg_type() {
  set_has_msg_type();
  // @@protoc_insertion_point(field_mutable:db.ClusterRequest.msg_type)
  return msg_type_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ClusterRequest::release_msg_type() {
  // @@protoc_insertion_point(field_release:db.ClusterRequest.msg_type)
  clear_has_msg_type();
  return msg_type_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClusterRequest::set_allocated_msg_type(::std::string* msg_type) {
  if (msg_type != NULL) {
    set_has_msg_type();
  } else {
    clear_has_msg_type();
  }
  msg_type_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg_type);
  // @@protoc_insertion_point(field_set_allocated:db.ClusterRequest.msg_type)
}

// required string msg = 2;
inline bool ClusterRequest::has_msg() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ClusterRequest::set_has_msg() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ClusterRequest::clear_has_msg() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ClusterRequest::clear_msg() {
  msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_msg();
}
inline const ::std::string& ClusterRequest::msg() const {
  // @@protoc_insertion_point(field_get:db.ClusterRequest.msg)
  return msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClusterRequest::set_msg(const ::std::string& value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:db.ClusterRequest.msg)
}
inline void ClusterRequest::set_msg(const char* value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:db.ClusterRequest.msg)
}
inline void ClusterRequest::set_msg(const char* value, size_t size) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:db.ClusterRequest.msg)
}
inline ::std::string* ClusterRequest::mutable_msg() {
  set_has_msg();
  // @@protoc_insertion_point(field_mutable:db.ClusterRequest.msg)
  return msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ClusterRequest::release_msg() {
  // @@protoc_insertion_point(field_release:db.ClusterRequest.msg)
  clear_has_msg();
  return msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClusterRequest::set_allocated_msg(::std::string* msg) {
  if (msg != NULL) {
    set_has_msg();
  } else {
    clear_has_msg();
  }
  msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg);
  // @@protoc_insertion_point(field_set_allocated:db.ClusterRequest.msg)
}

// -------------------------------------------------------------------

// ClusterResponse

// required string msg = 1;
inline bool ClusterResponse::has_msg() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ClusterResponse::set_has_msg() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ClusterResponse::clear_has_msg() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ClusterResponse::clear_msg() {
  msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_msg();
}
inline const ::std::string& ClusterResponse::msg() const {
  // @@protoc_insertion_point(field_get:db.ClusterResponse.msg)
  return msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClusterResponse::set_msg(const ::std::string& value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:db.ClusterResponse.msg)
}
inline void ClusterResponse::set_msg(const char* value) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:db.ClusterResponse.msg)
}
inline void ClusterResponse::set_msg(const char* value, size_t size) {
  set_has_msg();
  msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:db.ClusterResponse.msg)
}
inline ::std::string* ClusterResponse::mutable_msg() {
  set_has_msg();
  // @@protoc_insertion_point(field_mutable:db.ClusterResponse.msg)
  return msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ClusterResponse::release_msg() {
  // @@protoc_insertion_point(field_release:db.ClusterResponse.msg)
  clear_has_msg();
  return msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ClusterResponse::set_allocated_msg(::std::string* msg) {
  if (msg != NULL) {
    set_has_msg();
  } else {
    clear_has_msg();
  }
  msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), msg);
  // @@protoc_insertion_point(field_set_allocated:db.ClusterResponse.msg)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace db

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_db_2eproto__INCLUDED
