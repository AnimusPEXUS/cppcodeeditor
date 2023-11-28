
#include <memory>
#include <tuple>

#include "../../CommonEditorWindow.hpp"
#include "CodeEditorCCPP.hpp"
#include "mod_ccpp.hpp"

namespace wayround_i2p
{
namespace codeeditor
{

    CodeEditorMod mod_info_ccpp = {
        name : "CCPP",
        title : "C/C++",
        description : "C/C++ editor",
        supported_extensions : {".cpp", ".c", ".cc", ".hpp", ".h", ".hh"},
        newEditorForSubject : &CodeEditorCCPP::create
    };

    CodeEditorMod *get_mod_info_ccpp()
    {
        return &mod_info_ccpp;
    }

} // namespace codeeditor
} // namespace wayround_i2p
