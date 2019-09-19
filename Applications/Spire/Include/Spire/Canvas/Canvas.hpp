#ifndef SPIRE_CANVAS_HPP
#define SPIRE_CANVAS_HPP

namespace Spire {
  class AbsNode;
  class AdditionNode;
  class AggregateNode;
  class AlarmNode;
  class BaseValueNode;
  class BboQuoteNode;
  class BlotterTaskMonitorNode;
  class BooleanNode;
  class BooleanType;
  class BreadthFirstCanvasNodeIterator;
  class CanvasNode;
  class CanvasNodeBuilder;
  struct CanvasNodeNameSuffix;
  class CanvasNodeTask;
  class CanvasNodeTranslationContext;
  class CanvasNodeValidationError;
  class CanvasNodeVisitor;
  class CanvasObserver;
  class CanvasOperationException;
  class CanvasPath;
  class CanvasType;
  class CanvasTypeCompatibilityException;
  class CanvasTypeNotFoundException;
  class CanvasTypeRedefinitionException;
  class CanvasTypeRegistry;
  class CanvasTypeVisitor;
  class CeilNode;
  class ChainNode;
  class CurrencyNode;
  class CurrencyType;
  class CurrentDateNode;
  class CurrentDateTimeNode;
  class CurrentTimeNode;
  class CustomNode;
  class DateTimeNode;
  class DateTimeType;
  class DecimalNode;
  class DecimalType;
  class DefaultCurrencyNode;
  class DestinationNode;
  class DestinationType;
  class DivisionNode;
  class DurationNode;
  class DurationType;
  class EqualsNode;
  class ExecutionReportMonitorNode;
  class FilePathNode;
  class FileReaderNode;
  class FilterNode;
  class FloorNode;
  class FirstNode;
  class FoldNode;
  class FoldOperandNode;
  class FunctionNode;
  class GreaterNode;
  class GreaterOrEqualsNode;
  class IfNode;
  class IntegerNode;
  class IntegerType;
  class InteractionsNode;
  class IsTerminalNode;
  class LastNode;
  class LesserNode;
  class LesserOrEqualsNode;
  class LuaScriptNode;
  class LinkedNode;
  class MarketNode;
  class MarketType;
  class MaxFloorNode;
  class MaxNode;
  class MinNode;
  class MoneyNode;
  class MoneyType;
  class MultiplicationNode;
  class NativeType;
  class NoneNode;
  class NotNode;
  class OptionalPriceNode;
  class OrderExecutionPublisherTaskFactory;
  class OrderImbalanceQueryNode;
  class OrderStatusNode;
  class OrderStatusType;
  class OrderTypeNode;
  class OrderTypeType;
  class OrderWrapperTaskNode;
  class ProxyNode;
  class QueryNode;
  class RangeNode;
  class ReactorMonitor;
  class Record;
  class RecordLuaReactorParameter;
  class RecordNode;
  class RecordParser;
  class RecordType;
  class ReferenceNode;
  class RoundNode;
  class SecurityNode;
  class SecurityPortfolioNode;
  class SecurityType;
  class SideNode;
  class SideType;
  class SignatureNode;
  class SingleOrderTaskNode;
  class SpawnNode;
  class SubtractionNode;
  class Task;
  class TaskType;
  class TaskStateMonitorNode;
  class TaskStateNode;
  class TaskStateType;
  struct TaskTranslation;
  class TextNode;
  class TextType;
  class TimeAndSaleQueryNode;
  class TimeInForceNode;
  class TimeInForceType;
  class TimeNode;
  class TimeRangeNode;
  class TimeRangeParameterNode;
  class TimeRangeType;
  class TimerNode;
  class UnequalNode;
  class UnionType;
  class UntilNode;
  template<typename T> class ValueNode;
  class WhenNode;
}

#endif
