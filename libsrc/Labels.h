///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2008 Gabriel Yoder
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////
#ifndef LABELS_H
#define LABELS_H

#define BACKUP_STR                              "Backup"
#define BINARY_COMPATABILITY_REVISION_STR       "BinaryCompatabilityRevision"
#define BUILD_STR                               "Build"
#define CHECKSUM_STR                            "Checksum"
#define CONDITION_STR                           "Condition"
#define CONFIGURE_STR                           "Configure"
#define DEPENDENCY_STR                          "Dependency"
#define DEPENDENT_NAME_STR                      "DependentName"
#define DEPENDENT_REVISION_STR                  "DependentRevision"
#define DIR_STR                                 "Directory"
#define ENABLED_STR                             "Enabled"
#define ENTRY_STR                               "Entry"
#define ERROR_MSG_STR                           "ErrorMessage"
#define EXCLUDE_STR                             "ExcludeFromUpdates"
#define EXPLICIT_INSTALL_STR                    "ExplicitInstall"
#define FEATURE_NAME_STR                        "FeatureName"
#define FEATURE_STR                             "FeatureOption"
#define FILE_STR                                "File"
#define INHERITED_STR                           "Inherited"
#define INSTALL_FILE_STR                        "InstallFileDirectory"
#define INSTALL_LIST_FILE_STR                   "InstallListFile"
#define INSTALL_STR                             "Install"
#define INSTRUCTION_STR                         "Instruction"
#define INSTRUCTION_TYPE_STR                    "InstructionType"
#define LOCATION_STR                            "Location"
#define LOG_STR                                 "InstallLog"
#define MAX_REVISION_STR                        "MaxRevision"
#define PACKAGE_NAME_STR                        "PackageName"
#define PACKAGE_INDEX_STR                       "PackageIndex"
#define PROCESSOR_STR                           "Processors"
#define REPOSITORY_STR                          "Repository"
#define REVERSE_DEPENDENCY_STR                  "ReverseDependency"
#define REVISION_STR                            "Revision"
#define SHELL_STR                               "Shell"
#define SLOT_STR                                "Slot"
#define SOURCE_FILE_STR                         "SourceFile"
#define STATUS_STR                              "Status"
#define STORAGE_STR                             "StorageDirectory"
#define SYM_LINK_STR                            "SymbolicLink"
#define SYNC_PROTOCOL_STR                       "SyncProtocol"
#define SYNC_LOCATION_STR                       "SyncLocation"
#define TEXT_STR                                "Text"
#define TYPE_STR                                "Type"
#define UNINSTALL_STR                           "Uninstall"
#define UNPACK_STR                              "Unpack"
#define VERSION_STR                             "Version"
#define WORK_STR                                "WorkDirectory"


#define PACKAGE_HDR_STR         "AppRoverPackage"
#define CONFIG_HDR_STR          "AppRoverConfig"
#define LOG_HDR_STR             "AppRoverInstallLog"
#define REPOSITORY_HDR_STR      "AppRoverRepository"
#define LIST_HDR_STR            "AppRoverFileList"

#define STATUS_UNPACKED_STR     "unpacked"
#define STATUS_CONFIGURED_STR   "configured"
#define STATUS_BUILT_STR        "built"
#define STATUS_INSTALLED_STR    "installed"

#define DEP_TYPE_STATIC         "static"
#define DEP_TYPE_DYNAMIC        "dynamic"
#define DEP_TYPE_INSTALL        "install"

#define SLASH_STR                       "/"
#define COPY_STR                        "cp "
#define MOVE_STR                        "mv "
#define REMOVE_STR                      "rm -f "
#define REMOVE_DIR_STR                  "rmdir "
#define PACKAGE_FILE_EXTENSION_STR      ".arp"
#define REPOSITORY_CONFIG_FILE_STR      "repository.cfg"
#endif
