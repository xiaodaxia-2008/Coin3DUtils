import glob
import logging
import os
from pathlib import Path

import litgen

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def my_litgen_options() -> litgen.LitgenOptions:
    # configure your options here
    options = litgen.LitgenOptions()

    options.original_location_flag_show = False
    options.original_location_nb_parent_folders = 2
    options.original_signature_flag_show = True
    options.python_convert_to_snake_case = False

    # ///////////////////////////////////////////////////////////////////
    #  Root namespace
    # ///////////////////////////////////////////////////////////////////
    # The namespace DaftLib is the C++ root namespace for the generated bindings
    # (i.e. no submodule will be generated for it in the python bindings)
    options.namespaces_root = ["PyInventor"]
    options.type_replacements.add_last_replacement(r"\bunsigned \s*char\b", "int")

    # //////////////////////////////////////////////////////////////////
    # Basic functions bindings
    # ////////////////////////////////////////////////////////////////////
    # No specific option is needed for these basic bindings
    # litgen will add the docstrings automatically in the python bindings

    # //////////////////////////////////////////////////////////////////
    # Classes and structs bindings
    # //////////////////////////////////////////////////////////////////
    # No specific option is needed for these bindings.
    # - Litgen will automatically add a default constructor with named parameters
    #   for structs that have no constructor defined in C++.
    #  - A class will publish only its public methods and members

    # ///////////////////////////////////////////////////////////////////
    #  Exclude functions and/or parameters from the bindings
    # ///////////////////////////////////////////////////////////////////
    # We want to exclude `inline void priv_SetOptions(bool v) {}` from the bindings
    # priv_ is a prefix for private functions that we don't want to expose
    options.fn_exclude_by_name__regex = "^getDC$|^getPathCode$|^usePathCode$|^getAbortCallback$"

    # Inside `inline void SetOptions(bool v, bool priv_param = false) {}`,
    # we don't want to expose the private parameter priv_param
    # (it is possible since it has a default value)
    options.fn_params_exclude_names__regex = "^priv_"

    # ////////////////////////////////////////////////////////////////////
    # Override virtual methods in python
    # ////////////////////////////////////////////////////////////////////
    # The virtual methods of this class can be overriden in python
    # options.class_override_virtual_methods_in_python__regex = "^Animal$"

    # ////////////////////////////////////////////////////////////////////
    # Publish bindings for template functions
    # ////////////////////////////////////////////////////////////////////
    #  template<typename T> T MaxValue(const std::vector<T>& values);
    # will be published as: max_value_int and max_value_float
    # options.fn_template_options.add_specialization("^MaxValue$", ["int", "float"], add_suffix_to_function_name=True)
    #  template<typename T> T MaxValue(const std::vector<T>& values);
    # will be published as: max_value_int and max_value_float
    # options.fn_template_options.add_specialization("^MinValue$", ["int", "float"], add_suffix_to_function_name=False)

    # ////////////////////////////////////////////////////////////////////
    # Return values policy
    # ////////////////////////////////////////////////////////////////////
    # `Widget& GetWidgetSingleton()` returns a reference, that python should not free,
    # so we force the reference policy to be 'reference' instead of 'automatic'
    # options.fn_return_force_policy_reference_for_references__regex = "Singleton$"

    # ////////////////////////////////////////////////////////////////////
    #  Boxed types
    # ////////////////////////////////////////////////////////////////////
    # Adaptation for `inline void SwitchBoolValue(bool &v) { v = !v; }`
    # SwitchBoolValue is a C++ function that takes a bool parameter by reference and changes its value
    # Since bool are immutable in python, we can to use a BoxedBool instead
    options.fn_params_replace_modifiable_immutable_by_boxed__regex = r".*"

    # ////////////////////////////////////////////////////////////////////
    #  Published vectorized math functions and namespaces
    # ////////////////////////////////////////////////////////////////////
    # The functions in the MathFunctions namespace will be also published as vectorized functions
    # options.fn_namespace_vectorize__regex = r"^DaftLib::MathFunctions$"  # Do it in this namespace only
    # options.fn_vectorize__regex = r".*"  # For all functions

    # ////////////////////////////////////////////////////////////////////
    # Format the python stubs with black
    # ////////////////////////////////////////////////////////////////////
    # Set to True if you want the stub file to be formatted with black
    options.python_run_black_formatter = True

    return options


def autogenerate(include_dir: Path, include_base: Path, output_dir: Path) -> None:
    # header_files = glob.glob(str(include_dir) + "/*.h", recursive=False)

    # logger.debug(f"header fiels:\n{'\n'.join(header_files)}")

    for path in include_dir.iterdir():
        if path.is_dir():
            autogenerate(path, include_base, output_dir)
        elif path.is_file() and path.suffix == ".h":
            header_file = path
            header_files = [header_file.as_posix()]
            rel_path = Path(os.path.relpath(header_file, str(include_base)))
            name = path.stem.capitalize()
            output_cpp_pydef_file = (
                output_dir / f"_pydef_pybind11/{rel_path.with_suffix('.cpp')}"
            )
            os.makedirs(output_cpp_pydef_file.parent, exist_ok=True)
            with open(output_cpp_pydef_file, "w") as f:
                f.write(f"#include <{rel_path.as_posix()}>\n")
                f.write(
                    f"""
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void Bind{name}(py::module &m) {{

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AUTOGENERATED CODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// <litgen_glue_code>  // Autogenerated code below! Do not edit!
// </litgen_glue_code> // Autogenerated code end
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AUTOGENERATED CODE END !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AUTOGENERATED CODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// <litgen_pydef>  // Autogenerated code below! Do not edit!
// </litgen_pydef> // Autogenerated code end
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AUTOGENERATED CODE END !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

}}
"""
                )

            output_stub_pyi_file = output_dir / f"_stubs/{rel_path.with_suffix('.pyi')}"
            os.makedirs(output_stub_pyi_file.parent, exist_ok=True)
            with open(output_stub_pyi_file, "a") as f:
                f.write(
                    """import enum
from typing import overload

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AUTOGENERATED CODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# <litgen_stub> // Autogenerated code below! Do not edit!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AUTOGENERATED CODE END !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# </litgen_stub> // Autogenerated code end!
"""
                )

            litgen.write_generated_code_for_files(
                options=my_litgen_options(),
                input_cpp_header_files=header_files,
                output_cpp_pydef_file=output_cpp_pydef_file.as_posix(),
                output_stub_pyi_file=output_stub_pyi_file.as_posix(),
            )


if __name__ == "__main__":
    repository_dir = Path(__file__).parent.parent

    include_base = repository_dir / "build/vcpkg_installed/x64-windows/include"
    include_dir = include_base / "Inventor/actions"

    autogenerate(include_dir, include_base, repository_dir / "PyInventor")
