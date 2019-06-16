USE WebVisu;
-- Pages
CREATE TABLE IF NOT EXISTS `Pages` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ParentID` bigint(20) unsigned DEFAULT NULL,
  `title` varchar(30) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `Pages_Pages_FK` (`ParentID`),
  CONSTRAINT `Pages_Pages_FK` FOREIGN KEY (`ParentID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsTypes
CREATE TABLE IF NOT EXISTS `GuiElementsTypes` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` varchar(32) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElements
CREATE TABLE IF NOT EXISTS `GuiElements` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `PageID` bigint(20) unsigned DEFAULT NULL,
  `TypeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElements_Pages_FK` (`PageID`),
  KEY `GuiElements_GuiElementsTypes_FK` (`TypeID`),
  CONSTRAINT `GuiElements_GuiElementsTypes_FK` FOREIGN KEY (`TypeID`) REFERENCES `GuiElementsTypes` (`ID`),
  CONSTRAINT `GuiElements_Pages_FK` FOREIGN KEY (`PageID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- DataTypes
CREATE TABLE `DataTypes` (
  `type` varchar(100) NOT NULL,
  PRIMARY KEY (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsDataNodesTemplate
CREATE TABLE `GuiElementsDataNodesTemplate` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` varchar(100) NOT NULL,
  `defaultValue` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementsDataNodesTemplate_FK` (`type`),
  CONSTRAINT `GuiElementsDataNodesTemplate_FK` FOREIGN KEY (`type`) REFERENCES `DataTypes` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypesDataNodesRel
CREATE TABLE `GuiElementTypesDataNodesRel` (
  `GuiElementTypeID` int(10) unsigned NOT NULL,
  `DataNodeTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`DataNodeTemplateID`),
  KEY `GuiElementTypesDataNodesRel_FK_1` (`DataNodeTemplateID`),
  CONSTRAINT `GuiElementTypesDataNodesRel_FK` FOREIGN KEY (`GuiElementTypeID`) REFERENCES `GuiElementsTypes` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesDataNodesRel_FK_1` FOREIGN KEY (`DataNodeTemplateID`) REFERENCES `GuiElementsDataNodesTemplate` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsDataNodes
CREATE TABLE `GuiElementsDataNodes` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GuiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementsDataNodes_GuiElements_FK` (`GuiElementID`),
  KEY `GuiElementsDataNodes_FK` (`typeID`),
  CONSTRAINT `GuiElementsDataNodes_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementsDataNodesTemplate` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementsDataNodes_GuiElements_FK` FOREIGN KEY (`GuiElementID`) REFERENCES `GuiElements` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsParamsTemplate
CREATE TABLE `GuiElementsParamsTemplate` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` varchar(100) NOT NULL,
  `defaultValue` varchar(100) DEFAULT NULL,
  `description` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementsParamsTemplate_FK` (`type`),
  CONSTRAINT `GuiElementsParamsTemplate_FK` FOREIGN KEY (`type`) REFERENCES `DataTypes` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypesParamRel
CREATE TABLE `GuiElementTypesParamRel` (
  `GuiElementTypeID` int(10) unsigned NOT NULL,
  `ParamTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`ParamTemplateID`),
  KEY `GuiElementTypesParamRel_FK` (`ParamTemplateID`),
  CONSTRAINT `GuiElementTypesParamRel_FK` FOREIGN KEY (`ParamTemplateID`) REFERENCES `GuiElementsParamsTemplate` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesParamRel_FK_1` FOREIGN KEY (`GuiElementTypeID`) REFERENCES `GuiElementsTypes` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsParams
CREATE TABLE `GuiElementsParams` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GuiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementsParams_GuiElements_FK` (`GuiElementID`),
  KEY `GuiElementsParams_FK` (`typeID`),
  CONSTRAINT `GuiElementsParams_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementsParamsTemplate` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementsParams_GuiElements_FK` FOREIGN KEY (`GuiElementID`) REFERENCES `GuiElements` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO GuiElementsTypes (`type`) VALUES
	('button'),
	('numInput'),
	('colorIndicator'),
	('label');

Insert INTO Pages (ParentID, title) values
	(NULL,'ROOT_PAGE'),
	(1,'SUB_PAGE');

INSERT INTO DataTypes (type) values
	('Bool'),
	('Int8'),
	('Int16'),
	('Int32'),
	('Int64'),
	('UInt8'),
	('UInt16'),
	('UInt32'),
	('UInt64'),
	('Float'),
	('Double'),
	('String'),
	('DateTime');
INSERT INTO GuiElementsParamsTemplate (type, defaultValue, description)
