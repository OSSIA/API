#! /usr/bin/env python
# -*- coding: utf-8 -*-

"""
This ia a test for pyossia module, based on libossia python bindings
"""

import unittest

# import pyossia module

import pyossia as ossia

# Python 2 compatibility
import sys
if sys.version_info[0] < 3:
    unittest.TestCase.assertCountEqual = unittest.TestCase.assertItemsEqual
try:
    # python 2
    str = unicode
    def is_string(test):
        return isinstance(test, basestring)
except NameError:
    # python 3
    def is_string(test):
        return isinstance(test, str)


class TestAll(unittest.TestCase):
    """
    Main test, it will trigger all the tests
    """
    # create the OSSIA Device
    my_device = ossia.LocalDevice('PyOssia Test Device')
    # expose this device to the network
    # First with oscquery protocol
    my_device.expose(protocol='oscquery', listening_port=3456, sending_port=5678, logger=False)
    # then through OSC
    #my_device.expose(protocol='osc', listening_port=9996, sending_port=9997, logger=False)
    # create a bunch of parameters under different nodes
    my_int = my_device.add_param('int', value_type='int', default_value=66, domain=[-100, 100])
    my_float = my_device.add_param('float', value_type='float', default_value=0.123456789, domain=[-2, 2])
    my_bool = my_device.add_param('special/bool', value_type='bool', default_value=True)
    my_string = my_device.add_param('string',value_type='string', default_value='Hello world !', domain=['once', 'loop'])
    my_vec2f = my_device.add_param('list/vec2f', value_type='vec2f', default_value=[0.5, 0.5],  domain=[0, 1])
    my_vec3f = my_device.add_param('list/vec3f', value_type='vec3f', default_value=[-270, 180, 45],  domain=[0, 360])
    my_vec4f = my_device.add_param('list/vec4f', value_type='vec4f', default_value=[0, 146, 207, 127],  domain=[0, 255])
    my_list = my_device.add_param('list/list', value_type='list', default_value=[44100, "my_track.wav", 0.6])
    my_char = my_device.add_param('special/char', value_type='char', default_value=chr(97))

    my_device.root_node.init()

    def test_version(self):
        """
        print the actual version of pyossia
        """
        self.assertEqual(isinstance(ossia.__version__, str), True)

    def test_int(self):
        """
        test a parameter @value_type float
        """
        self.assertEqual(self.my_int.value, 66)
        self.assertEqual(self.my_int.value_type, ossia.ValueType.Int)
        self.assertEqual(self.my_int.have_domain(), True)
        self.assertEqual(self.my_int.domain.min, -100)
        self.assertEqual(self.my_int.domain.max, 100)
        self.my_int.value = 60
        self.assertAlmostEqual(self.my_int.value, 60)

    def test_float(self):
        """
        test a parameter @value_type float
        """
        self.assertAlmostEqual(self.my_float.value, 0.123456789)
        self.assertEqual(self.my_float.value_type, ossia.ValueType.Float)
        self.assertEqual(self.my_float.have_domain(), True)
        self.assertEqual(self.my_float.domain.min, -2.0)
        self.assertEqual(self.my_float.domain.max, 2.0)
        self.my_float.value = 0.123456
        self.assertAlmostEqual(self.my_float.value, 0.123456)

    def test_char(self):
        self.assertEqual(self.my_char.value, 'a')
        self.my_char.value = '@'
        self.assertEqual(self.my_char.value, '@')
        self.my_char.value = 'Kj'
        self.assertEqual(self.my_char.value, '\x00')
        self.my_char.value = '@'
        self.assertEqual(self.my_char.value, '@')

    def test_string(self):
        """
        test a parameter @value_type float
        """
        self.assertEqual(isinstance(self.my_string.value, str), True)
        self.assertEqual(self.my_string.value, 'Hello world !')
        self.assertEqual(self.my_string.value_type, ossia.ValueType.String)
        self.my_string.value = 'What a new string ?'
        self.assertEqual(self.my_string.value, 'What a new string ?')

    def test_vec2f(self):
        """
        test a parameter @value_type float
        """
        self.assertEqual(len(self.my_vec2f.value), 2)
        self.assertCountEqual(self.my_vec2f.value, [0.5, 0.5])
        self.assertEqual(self.my_vec2f.value_type, ossia.ValueType.Vec2f)
        self.assertEqual(self.my_vec2f.have_domain(), True)
        self.my_vec2f.value = [-1, 1.2]
        self.assertAlmostEqual(self.my_vec2f.value[0], -1)
        self.assertAlmostEqual(self.my_vec2f.value[1], 1.2)
        self.assertEqual(len(self.my_vec2f.value), 2)

    def test_vec3f(self):
        """
        test a parameter @value_type float
        """
        self.assertCountEqual(self.my_vec3f.value, [-270, 180, 45])
        self.assertEqual(len(self.my_vec3f.value), 3)
        self.assertEqual(self.my_vec3f.value_type, ossia.ValueType.Vec3f)
        self.assertEqual(self.my_vec2f.have_domain(), True)
        self.my_vec3f.value = [-1, -270, 360]
        self.assertEqual(len(self.my_vec3f.value), 3)
        self.assertAlmostEqual(self.my_vec3f.value[0], -1)
        self.assertAlmostEqual(self.my_vec3f.value[1], -270)
        self.assertAlmostEqual(self.my_vec3f.value[2], 360)
        self.assertEqual(len(self.my_vec3f.value), 3)

    def test_vec4f(self):
        """
        test a parameter @value_type float
        """
        self.assertCountEqual(self.my_vec4f.value, [0, 146, 207, 127])
        self.assertEqual(len(self.my_vec4f.value), 4)
        self.assertEqual(self.my_vec4f.value_type, ossia.ValueType.Vec4f)
        self.assertEqual(self.my_vec2f.have_domain(), True)
        self.my_vec4f.value = [0, 0.6, 0.86, 0.8]
        self.assertAlmostEqual(self.my_vec4f.value[0], 0)
        self.assertAlmostEqual(self.my_vec4f.value[1], 0.6)
        self.assertAlmostEqual(self.my_vec4f.value[2], 0.86)
        self.assertAlmostEqual(self.my_vec4f.value[3], 0.8)
        self.assertEqual(len(self.my_vec4f.value), 4)

    def test_device(self):
        """
        test a device
        """
        # Test the class of the device
        self.assertEqual(self.my_device.__class__.__name__, 'LocalDevice')
        # Test the string representation of the device's root node
        self.assertEqual(str(self.my_device.root_node), '/')
        # Grab the name of the device
        self.assertEqual(self.my_device.name, 'PyOssia Test Device')
        # How many children this device have?
        self.assertEqual(len(self.my_device.root_node.children()), 5)
        # How many nodes are under this device?
        self.assertEqual(len(self.my_device.root_node.get_nodes()), 2)
        # How many parameters under this device?
        self.assertEqual(len(self.my_device.root_node.get_parameters()), 9)
        # How many parameters directly attached to root node?
        self.assertEqual(len(self.my_device.root_node.get_parameters(depth=1)), 3)
        # How many parameters directly attached to the first child of root_node?
        self.assertEqual(len(self.my_device.root_node.get_nodes()[0].get_parameters()), 2)
        # How many parameters directly attached to the second child of root_node?
        self.assertEqual(len(self.my_device.root_node.get_nodes()[1].get_parameters()), 4)
        # Testing NODES
        self.assertEqual(self.my_device.find_node('/special/bool'), self.my_bool.node)


if __name__ == '__main__':
    unittest.main()
