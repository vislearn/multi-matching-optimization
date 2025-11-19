"""Sphinx extension to prefer stub file docstrings over runtime docstrings for C++ bindings.

This extension ensures that documentation from .pyi stub files takes precedence
over any runtime docstrings for C++ pybind11 modules, while allowing pure Python
modules to use their natural runtime docstrings.
"""

import ast
from pathlib import Path


class StubDocstringLoader:
    """Load and cache docstrings from .pyi stub files."""
    
    def __init__(self):
        self.docstrings = {}
        self.types = {}
        self.overloads = {}  # Separate storage for overloaded function signatures
    
    def load(self, stub_dir):
        """Load docstrings from stub files in directory."""
        stub_files = {
            'pylibmgm': stub_dir / 'pylibmgm' / '__init__.pyi',
            'pylibmgm.io': stub_dir / 'pylibmgm' / 'io.pyi',
        }
        
        for module_name, stub_path in stub_files.items():
            if stub_path.exists():
                self._parse_stub_file(module_name, stub_path)
    
    def _parse_stub_file(self, module_name, stub_path):
        """Parse a stub file and extract docstrings."""
        with open(stub_path, 'r', encoding='utf-8') as f:
            tree = ast.parse(f.read())
        
        for node in tree.body:
            if isinstance(node, ast.ClassDef):
                self._extract_class_docs(module_name, node)
            elif isinstance(node, ast.FunctionDef):
                self._extract_function_docs(module_name, node)
    
    def _build_full_class_name(self, module_name, class_name, parent_class=None):
        """Build the fully qualified class name."""
        if parent_class:
            return f'{module_name}.{parent_class}.{class_name}'
        return f'{module_name}.{class_name}'
    
    def _is_property(self, func_node):
        """Check if a function node is decorated with @property."""
        return any(
            isinstance(d, ast.Name) and d.id == 'property'
            for d in func_node.decorator_list
        )
    
    def _extract_inline_docstring(self, class_node, index):
        """Extract inline docstring following an attribute definition.
        
        Parameters
        ----------
        class_node : ast.ClassDef
            The class node containing the attribute
        index : int
            Index of the attribute in the class body
            
        Returns
        -------
        str or None
            The inline docstring if present, otherwise None
        """
        if index + 1 < len(class_node.body):
            next_node = class_node.body[index + 1]
            if isinstance(next_node, ast.Expr) and isinstance(next_node.value, ast.Constant):
                if isinstance(next_node.value.value, str):
                    return next_node.value.value
        return None
    
    def _process_nested_class(self, module_name, class_name, parent_class, nested_node):
        """Process a nested class definition."""
        nested_parent = f'{parent_class}.{class_name}' if parent_class else class_name
        self._extract_class_docs(module_name, nested_node, parent_class=nested_parent)
    
    def _process_method(self, full_class_name, method_node):
        """Process a method or property definition."""
        method_name = method_node.name
        method_full_name = f'{full_class_name}.{method_name}'
        method_doc = ast.get_docstring(method_node)
        
        is_overload = self._is_overload_decorator(method_node)
        is_property = self._is_property(method_node)
        
        if is_overload:
            self._store_overload_signature(method_full_name, method_node, method_doc)
        elif method_doc:
            # For properties, store only summary; for methods, store full docstring
            if is_property:
                self.docstrings[method_full_name] = self._extract_summary(method_doc)
            else:
                self.docstrings[method_full_name] = method_doc
        
        # Store property type annotation
        if is_property and method_node.returns:
            self.types[method_full_name] = self._annotation_to_string(method_node.returns)
    
    def _process_attribute(self, full_class_name, attr_node, class_node, index, attr_docs):
        """Process an attribute with type annotation."""
        if not isinstance(attr_node.target, ast.Name):
            return
        
        attr_name = attr_node.target.id
        attr_full_name = f'{full_class_name}.{attr_name}'
        
        # Store type annotation
        if attr_node.annotation:
            self.types[attr_full_name] = self._annotation_to_string(attr_node.annotation)
        
        # Try to extract inline docstring first
        inline_doc = self._extract_inline_docstring(class_node, index)
        if inline_doc:
            self.docstrings[attr_full_name] = self._extract_summary(inline_doc)
        elif attr_name in attr_docs:
            self.docstrings[attr_full_name] = attr_docs[attr_name]
    
    def _extract_class_docs(self, module_name, class_node, parent_class=None):
        """Extract documentation from a class definition.
        
        Parameters
        ----------
        module_name : str
            The module name
        class_node : ast.ClassDef
            The class AST node
        parent_class : str, optional
            Parent class name for nested classes
        """
        class_name = class_node.name
        full_class_name = self._build_full_class_name(module_name, class_name, parent_class)
        
        # Store class docstring
        docstring = ast.get_docstring(class_node)
        if docstring:
            self.docstrings[full_class_name] = docstring
        
        # Extract attribute docs from Napoleon-style Attributes section
        attr_docs = self._extract_napoleon_attrs(docstring) if docstring else {}
        
        # Process each class member
        for i, item in enumerate(class_node.body):
            if isinstance(item, ast.ClassDef):
                self._process_nested_class(module_name, class_name, parent_class, item)
            elif isinstance(item, ast.FunctionDef):
                self._process_method(full_class_name, item)
            elif isinstance(item, ast.AnnAssign):
                self._process_attribute(full_class_name, item, class_node, i, attr_docs)
    
    def _extract_function_docs(self, module_name, func_node):
        """Extract documentation from a function definition."""
        func_name = func_node.name
        full_name = f'{module_name}.{func_name}'
        docstring = ast.get_docstring(func_node)
        
        # Check if this is an overloaded function
        is_overload = self._is_overload_decorator(func_node)
        
        if is_overload:
            self._store_overload_signature(full_name, func_node, docstring)
        else:
            # Regular function - store docstring
            if docstring:
                self.docstrings[full_name] = docstring
    
    def _extract_summary(self, docstring):
        """Extract just the summary line from a docstring, excluding sections."""
        if not docstring:
            return ""
        
        lines = docstring.strip().split('\n')
        summary_lines = []
        
        for line in lines:
            stripped = line.strip()
            # Stop at section headers (Returns, Parameters, etc.)
            if stripped in ['Returns', 'Parameters', 'Raises', 'Notes', 'Examples', 'Attributes']:
                break
            # Stop at section underlines
            if stripped and all(c in '-=' for c in stripped):
                break
            # Add non-empty lines to summary
            if stripped or summary_lines:  # Allow empty lines within summary
                summary_lines.append(stripped)
        
        # Join and clean up
        summary = ' '.join(summary_lines).strip()
        return summary
    
    def _is_overload_decorator(self, func_node):
        """Check if a function has an @overload decorator."""
        return any(
            isinstance(d, ast.Name) and d.id == 'overload' or
            isinstance(d, ast.Attribute) and d.attr == 'overload'
            for d in func_node.decorator_list
        )
    
    def _store_overload_signature(self, full_name, func_node, docstring):
        """Store signature information for an overloaded function."""
        if full_name not in self.overloads:
            self.overloads[full_name] = []
        
        # Extract function signature
        args = []
        for arg in func_node.args.args:
            arg_name = arg.arg
            arg_type = self._annotation_to_string(arg.annotation) if arg.annotation else 'Any'
            args.append((arg_name, arg_type))
        
        return_type = self._annotation_to_string(func_node.returns) if func_node.returns else 'None'
        
        self.overloads[full_name].append({
            'args': args,
            'return_type': return_type,
            'docstring': docstring
        })
    
    def _extract_napoleon_attrs(self, docstring):
        """Extract attribute docs from Napoleon-style 'Attributes' section."""
        if not docstring:
            return {}
        
        attr_docs = {}
        lines = docstring.split('\n')
        in_attrs = False
        current_attr = None
        current_desc = []
        
        for line in lines:
            stripped = line.strip()
            
            # Detect start of Attributes section
            if stripped in ('Attributes', 'Attributes:'):
                in_attrs = True
                continue
            
            # Skip section underlines (--- or ===)
            if in_attrs and stripped and all(c in ('-', '=') for c in stripped):
                continue
            
            # Detect new section starting (non-indented line without colon)
            if in_attrs and stripped and line and not line[0].isspace() and ':' not in line:
                # Save current attribute and stop processing
                if current_attr and current_desc:
                    attr_docs[current_attr] = ' '.join(current_desc).strip()
                break
            
            if in_attrs:
                # New attribute definition (non-indented line with colon)
                if stripped and line and not line[0].isspace() and ':' in line:
                    # Save previous attribute
                    if current_attr and current_desc:
                        attr_docs[current_attr] = ' '.join(current_desc).strip()
                    # Start new attribute
                    current_attr = line.split(':', 1)[0].strip()
                    current_desc = []
                # Continuation of attribute description
                elif stripped and current_attr:
                    current_desc.append(stripped)
        
        # Save the last attribute
        if current_attr and current_desc:
            attr_docs[current_attr] = ' '.join(current_desc).strip()
        
        return attr_docs
    
    def _annotation_to_string(self, annotation):
        """Convert AST annotation node to string."""
        if annotation is None:
            return "None"
        elif isinstance(annotation, ast.Name):
            return annotation.id
        elif isinstance(annotation, ast.Attribute):
            return f"{self._annotation_to_string(annotation.value)}.{annotation.attr}"
        elif isinstance(annotation, ast.Subscript):
            value = self._annotation_to_string(annotation.value)
            # Unwrap ClassVar to get the actual type
            if value == 'typing.ClassVar':
                return self._annotation_to_string(annotation.slice)
            if isinstance(annotation.slice, ast.Tuple):
                args = ', '.join(self._annotation_to_string(e) for e in annotation.slice.elts)
            else:
                args = self._annotation_to_string(annotation.slice)
            return f"{value}[{args}]"
        elif isinstance(annotation, ast.Constant):
            return repr(annotation.value)
        return "Any"
    
    def _normalize_name(self, name):
        """Normalize C++ binding module names to stub file names.
        
        Core module functionality is moved from pylibmgm._pylibmgm.SOMETHING 
        to pylibmgm.SOMETHING in stub files.
        """
        if '._pylibmgm.' in name:
            return name.replace('._pylibmgm.', '.')
        return name
    
    def get_docstring(self, name):
        """Get docstring, trying common name variations."""
        normalized_name = self._normalize_name(name)
        return self.docstrings.get(normalized_name)
    
    def get_type(self, name):
        """Get type annotation, trying common name variations."""
        normalized_name = self._normalize_name(name)
        return self.types.get(normalized_name, 'object')
    
    def get_overloads(self, name):
        """Get overload signatures for a function if it's overloaded."""
        normalized_name = self._normalize_name(name)
        return self.overloads.get(normalized_name)


# Global loader instance
_loader = StubDocstringLoader()


def process_docstring(app, what, name, obj, options, lines):
    """Replace runtime docstrings with stub file docstrings where available."""
    stub_doc = _loader.get_docstring(name)
    if stub_doc:
        lines.clear()
        lines.extend(stub_doc.split('\n'))


def setup(app):
    """Sphinx extension setup."""
    # Load stub files
    conf_dir = Path(app.confdir)
    stub_dir = conf_dir / '..' / '..' / 'mgm_python' / 'stubs'
    _loader.load(stub_dir.resolve())
    
    # Patch autosummary renderer to add custom Jinja functions
    from sphinx.ext.autosummary.generate import AutosummaryRenderer
    
    original_init = AutosummaryRenderer.__init__
    
    def patched_init(self, app):
        original_init(self, app)
        
        def get_attr_type(cls_name, attr_name):
            """Get type annotation for an attribute."""
            return _loader.get_type(f'{cls_name}.{attr_name}')
        
        def get_attr_doc(cls_name, attr_name):
            """Get docstring for an attribute."""
            return _loader.get_docstring(f'{cls_name}.{attr_name}')
        
        def get_overloads(fullname):
            """Get overload signatures for a function."""
            return _loader.get_overloads(fullname)
        
        def is_enum_class(fullname):
            """Check if a class is an enum (Python enum.Enum or pybind11 enum)."""
            try:
                import importlib
                import enum
                
                # Get the top-level module
                module_name = fullname.split('.')[0]
                module = importlib.import_module(module_name)
                
                # Navigate through the dotted path to get the object
                obj = module
                for part in fullname.split('.')[1:]:
                    obj = getattr(obj, part)
                
                # Check for Python enum (using EnumMeta/EnumType is cleaner than issubclass)
                if isinstance(obj, enum.EnumMeta):
                    return True
                
                # Check for pybind11 enum (not part of Python's enum hierarchy)
                # pybind11 enums have type 'pybind11_type' and expose __members__
                if type(obj).__name__ == 'pybind11_type' and hasattr(obj, '__members__'):
                    return True
                    
            except (ImportError, AttributeError):
                pass
            return False
        
        def napoleon_process(docstring):
            """Process docstring through Napoleon to convert NumPy style to field lists."""
            if not docstring:
                return ""
            from sphinx.ext.napoleon.docstring import NumpyDocstring
            config = app.config
            processed = str(NumpyDocstring(docstring, config, app, what='method'))
            return processed
        
        def get_nested_class_doc(cls_name, attr_name):
            """Get docstring for a nested class type."""
            attr_type = get_attr_type(cls_name, attr_name)
            # Check if this type is a nested class
            nested_class_name = f'{cls_name}.{attr_type}'
            return _loader.get_docstring(nested_class_name)
        
        def get_nested_class_attrs(cls_name, attr_name):
            """Get attributes of a nested class type."""
            attr_type = get_attr_type(cls_name, attr_name)
            nested_class_name = f'{cls_name}.{attr_type}'
            
            # Try to get attributes for this nested class
            attrs = []
            # We need to check the types dictionary for anything starting with nested_class_name
            for key in _loader.types.keys():
                if key.startswith(nested_class_name + '.'):
                    attr_key = key[len(nested_class_name) + 1:]
                    # Only get direct attributes, not nested ones
                    if '.' not in attr_key:
                        attrs.append({
                            'name': attr_key,
                            'type': _loader.get_type(key),
                            'doc': _loader.get_docstring(key) or ''
                        })
            return attrs
        
        def make_type_xref(type_str):
            """Convert a type string to a Sphinx cross-reference.
            
            Dynamically checks if types are documented in the project and creates
            cross-references for them. Automatically discovers submodules.
            
            Handles:
            - Simple types: Graph -> :class:`~pylibmgm.Graph`
            - Generic types: list[Graph] -> list[:class:`~pylibmgm.Graph`]
            - Nested generics: dict[tuple[int, int], GmModel]
            - Built-in types: int, str, bool, float (not linked)
            """
            import re
            import importlib
            import pkgutil
            
            # Built-in types that shouldn't be linked
            builtins = {'int', 'str', 'bool', 'float', 'dict', 'list', 'tuple', 'set', 
                       'Any', 'None', 'Optional', 'Union', 'Callable', 'Iterable',
                       'Sequence', 'Mapping'}
            
            def get_submodules(package_name):
                """Discover all submodules of a package."""
                try:
                    package = importlib.import_module(package_name)
                    submodules = []
                    if hasattr(package, '__path__'):
                        for importer, modname, ispkg in pkgutil.iter_modules(package.__path__):
                            submodules.append(modname)
                    return submodules
                except (ImportError, AttributeError):
                    return []
            
            def is_documented_type(type_name):
                """Check if a type is documented in pylibmgm."""
                if type_name in builtins:
                    return False
                
                # Try to import from pylibmgm
                try:
                    module = importlib.import_module('pylibmgm')
                    if hasattr(module, type_name):
                        return True
                    
                    # Dynamically check all submodules
                    for submod in get_submodules('pylibmgm'):
                        try:
                            submodule = importlib.import_module(f'pylibmgm.{submod}')
                            if hasattr(submodule, type_name):
                                return True
                        except ImportError:
                            pass
                except ImportError:
                    pass
                
                return False
            
            def get_full_reference(type_name):
                """Get the full reference path for a type."""
                try:
                    # First try main module
                    module = importlib.import_module('pylibmgm')
                    if hasattr(module, type_name):
                        obj = getattr(module, type_name)
                        # Get the actual module where it's defined
                        if hasattr(obj, '__module__'):
                            return f'{obj.__module__}.{type_name}'
                        return f'pylibmgm.{type_name}'
                    
                    # Dynamically check all submodules
                    for submod in get_submodules('pylibmgm'):
                        try:
                            submodule = importlib.import_module(f'pylibmgm.{submod}')
                            if hasattr(submodule, type_name):
                                return f'pylibmgm.{submod}.{type_name}'
                        except ImportError:
                            pass
                except ImportError:
                    pass
                
                return f'pylibmgm.{type_name}'
            
            # Find all potential type names (capitalized words)
            # Match word-boundary capitalized identifiers
            result = type_str
            words = re.findall(r'\b[A-Z][a-zA-Z0-9_]*\b', type_str)
            
            for word in words:
                if is_documented_type(word):
                    full_ref = get_full_reference(word)
                    # Use ~prefix to show only the class name without module path
                    pattern = r'\b' + re.escape(word) + r'\b'
                    result = re.sub(pattern, f':class:`~{full_ref}`', result)
            
            return result
        
        self.env.globals['attr_type'] = get_attr_type
        self.env.globals['attr_doc'] = get_attr_doc
        self.env.globals['get_overloads'] = get_overloads
        self.env.globals['is_enum_class'] = is_enum_class
        self.env.globals['get_nested_class_doc'] = get_nested_class_doc
        self.env.globals['get_nested_class_attrs'] = get_nested_class_attrs
        self.env.globals['make_type_xref'] = make_type_xref
        self.env.filters['napoleon'] = napoleon_process
    
    AutosummaryRenderer.__init__ = patched_init
    
    # Connect hooks - use priority 100 so we run before Napoleon (which uses default 500)
    app.connect('autodoc-process-docstring', process_docstring, priority=100)
    
    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
