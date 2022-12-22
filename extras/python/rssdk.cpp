#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rserrors.h>

#include "pyrsdio.h"
#include "pyrspoe.h"

namespace py = pybind11;

PYBIND11_MODULE(rssdk, module)
{
    module.doc() =
        "Rugged Science SDK for monitoring and controlling DIO and PoE";

    py::enum_<rs::OutputMode>(module, "OutputMode")
        .value("Source", rs::OutputMode::Source)
        .value("Sink", rs::OutputMode::Sink);

    py::class_<PyRsDio>(module, "RsDio")
        .def(py::init<>())
        .def(
            "setXmlFile",
            &PyRsDio::setXmlFile,
            "Set hardware XML file",
            py::arg("fileName")
        )
        .def(
            "setOutputMode",
            &PyRsDio::setOutputMode,
            "Set the output mode of a single DIO bank",
            py::arg("dio"),
            py::arg("mode")
        )
        .def(
            "digitalRead",
            &PyRsDio::digitalRead,
            "Read the state of a single DIO pin",
            py::arg("dio"),
            py::arg("pin")
        )
        .def(
            "digitalWrite",
            &PyRsDio::digitalWrite,
            "Set the state of a single DIO pin",
            py::arg("dio"),
            py::arg("pin"),
            py::arg("state")
        )
        .def(
            "readAll",
            &PyRsDio::readAll,
            "Read the state of all pins on the specified DIO bank",
            py::arg("dio")
        );

    py::enum_<rs::PoeState>(module, "PoeState")
        .value("Disabled", rs::PoeState::Disabled)
        .value("Enabled", rs::PoeState::Enabled)
        .value("Auto", rs::PoeState::Auto);

    py::class_<PyRsPoe>(module, "RsPoe")
        .def(py::init<>())
        .def(
            "setXmlFile",
            &PyRsPoe::setXmlFile,
            "Set hardware XML file",
            py::arg("fileName")
        )
        .def(
            "getPortState",
            &PyRsPoe::getPortState,
            "Get the state of port",
            py::arg("port")
        )
        .def(
            "setPortState",
            &PyRsPoe::setPortState,
            "Set the state of port",
            py::arg("port"),
            py::arg("state")
        )
        .def(
            "getPortVoltage",
            &PyRsPoe::getPortVoltage,
            "Get the voltage output of port in volts",
            py::arg("port")
        )
        .def(
            "getPortCurrent",
            &PyRsPoe::getPortCurrent,
            "Get the current output of port in amps",
            py::arg("port")
        )
        .def(
            "getPortPower",
            &PyRsPoe::getPortPower,
            "Get the power output of port in watts",
            py::arg("port")
        )
        .def(
            "getBudgetConsumed",
            &PyRsPoe::getBudgetConsumed,
            "Get the current consumed budget of all ports in watts"
        )
        .def(
            "getBudgetAvailable",
            &PyRsPoe::getBudgetAvailable,
            "Get the current remaining budget for all ports in watts"
        )
        .def(
            "getBudgetTotal",
            &PyRsPoe::getBudgetTotal,
            "Get the max power in watts that all ports can consume"
        );

    py::register_local_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        }
        catch (const std::system_error &e) {
            const std::error_code &ec = e.code();
            if (ec == RsErrorCondition::PermissionError)
                PyErr_SetString(PyExc_PermissionError, e.what());
            else if (ec == RsErrorCondition::UnsupportedFunction)
                PyErr_SetString(PyExc_NotImplementedError, e.what());
            else if (ec == std::make_error_code(std::errc::invalid_argument))
                throw std::invalid_argument(e.what());
            else
                throw e;
        }
    });
}