#!/usr/bin/env python

import logging

LOGGER = None

##################################################
class LoggerDefaults:
    FORMATTER = logging.Formatter('%(asctime)s | %(levelname)7s | %(message)s', 
                                  '%Y-%m-%dT%H:%M:%S')
    LEVEL = logging.INFO

class Logger:
    ACTIVE = True
    LOGGER = None

    # Levels
    DEBUG = logging.DEBUG
    INFO = logging.INFO
    WARNING = logging.WARNING
    ERROR = logging.ERROR
    CRITICAL = logging.CRITICAL

    @staticmethod
    def setup(outfilename = '/dev/stderr', 
              formatter = LoggerDefaults.FORMATTER,
              level = LoggerDefaults.LEVEL,
              activate = True):
        if Logger.LOGGER is None:
            Logger.LOGGER = logging.getLogger()
        Logger.ACTIVE = activate
        logging_output = logging.FileHandler(outfilename)
        logging_output.setFormatter(formatter)
        Logger.LOGGER.addHandler(logging_output)
        Logger.LOGGER.setLevel(level)

    @staticmethod
    def setLevel(level):
        if Logger.LOGGER is None:
            Logger.setup()
        Logger.LOGGER.setLevel(level)

    @staticmethod
    def debug(message):
        if not Logger.ACTIVE:
            return 
        if Logger.LOGGER is None:
            Logger.setup()
        Logger.LOGGER.debug(message)

    @staticmethod
    def info(message):
        if not Logger.ACTIVE:
            return 
        if Logger.LOGGER is None:
            Logger.setup()
        Logger.LOGGER.info(message)

    @staticmethod
    def warn(message):
        Logger.warning(message)
    @staticmethod
    def warning(message):
        if not Logger.ACTIVE:
            return 
        if Logger.LOGGER is None:
            Logger.setup()
        Logger.LOGGER.warning(message)

    @staticmethod
    def error(message):
        Logger.LOGGER.error(message)
    @staticmethod
    def err(message):
        if Logger.LOGGER is None:
            Logger.setup()
        Logger.error(message)

    @staticmethod
    def critical(message):
        Logger.LOGGER.critical(message)
    @staticmethod
    def crit(message):
        if not Logger.ACTIVE:
            return 
        if Logger.LOGGER is None:
            Logger.setup()
        Logger.critical(message)
